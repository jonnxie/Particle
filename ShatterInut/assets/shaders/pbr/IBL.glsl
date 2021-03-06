#version 450

vec3 ibl(vec3 n, vec3 v, vec3 diffuseColor, vec3 f0, vec3 f90, float perceptualRoughness) {
    vec3 r = reflect(n);
    vec3 Ld = textureCube(irradianceEnvMap, r) * diffuseColor;
    float lod = computeLodFromRoughness(perceptualRoughness);
    vec3 Lld = textureCube(prefilteredEnvMap, r, lod);
    vec2 Ldfg = textureLod(dfgLut, vec2(dot(n, v), perceptualRoughness), 0.0).xy;
    vec3 Lr = (f0 * Ldfg.x + f90 * Ldfg.y) *Lld;
    return Ld + Lr;
}

vec3 irradianceSH(vec3 n) {
    // uniform vec3 sphericalHarmonics[9]
    // We can use only the first 2 bands for better performance
    return
    sphericalHarmonics[0]
    + sphericalHarmonics[1] * (n.y)
    + sphericalHarmonics[2] * (n.z)
    + sphericalHarmonics[3] * (n.x)
    + sphericalHarmonics[4] * (n.y * n.x)
    + sphericalHarmonics[5] * (n.y * n.z)
    + sphericalHarmonics[6] * (3.0 * n.z * n.z - 1.0)
    + sphericalHarmonics[7] * (n.z * n.x)
    + sphericalHarmonics[8] * (n.x * n.x - n.y * n.y);
}

// NOTE: this is the DFG LUT implementation of the function above
vec2 prefilteredDFG_LUT(float coord, float NoV) {
    // coord = sqrt(roughness), which is the mapping used by the
    // IBL prefiltering code when computing the mipmaps
    return textureLod(dfgLut, vec2(NoV, coord), 0.0).rg;
}

vec3 evaluateSpecularIBL(vec3 r, float perceptualRoughness) {
    // This assumes a 256x256 cubemap, with 9 mip levels
    float lod = 8.0 * perceptualRoughness;
    // decodeEnvironmentMap() either decodes RGBM or is a no-op if the
    // cubemap is stored in a float texture
    return decodeEnvironmentMap(textureCubeLodEXT(environmentMap, r, lod));
}

vec3 evaluateIBL(vec3 n, vec3 v, vec3 diffuseColor, vec3 f0, vec3 f90, float perceptualRoughness) {
    float NoV = max(dot(n, v), 0.0);
    vec3 r = reflect(-v, n);

    // Specular indirect
    vec3 indirectSpecular = evaluateSpecularIBL(r, perceptualRoughness);
    vec2 env = prefilteredDFG_LUT(perceptualRoughness, NoV);
    vec3 specularColor = f0 * env.x + f90 * env.y;

    // Diffuse indirect
    // We multiply by the Lambertian BRDF to compute radiance from irradiance
    // With the Disney BRDF we would have to remove the Fresnel term that
    // depends on NoL (it would be rolled into the SH). The Lambertian BRDF
    // can be baked directly in the SH to save a multiplication here
    vec3 indirectDiffuse = max(irradianceSH(n), 0.0) * Fd_Lambert();

    // Indirect contribution
    return diffuseColor * indirectDiffuse + indirectSpecular * specularColor;
}
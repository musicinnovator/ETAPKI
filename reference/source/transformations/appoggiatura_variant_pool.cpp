// Simple Appoggiatura Variant Pool - matches your existing structure
std::vector<std::pair<int, int>> applyVariant(int pi, int durPi, std::string& selectedVariant) {
    // Define the pool of appoggiatura variants - all options from the implementation
    const std::vector<std::string> variants = {
        // RPNB - Repeat Previous Note Below variants
        "RPNB",
        "RPNB.PREb1", "RPNB.PREb5", "RPNB.PREa1", "RPNB.PREa5",
        "RPNB.POSTb1", "RPNB.POSTb5", "RPNB.POSTa1", "RPNB.POSTa5",

        // RPNA - Repeat Previous Note Above variants
        "RPNA",
        "RPNA.PREb1", "RPNA.PREb5", "RPNA.PREa5", "RPNA.PREa1",
        "RPNA.POSTb1", "RPNA.POSTb5", "RPNA.POSTa5", "RPNA.POSTa1",

        // SAN - Step Above Note variants
        "SAN1", "SAN5",
        "SAN1.PREb1", "SAN5.PREb5", "SAN1.PREb5", "SAN5.PREb1",
        "SAN1.PREa1", "SAN5.PREa5", "SAN1.PREa5", "SAN5.PREa1",
        "SAN1.POSTa1", "SAN1.POSTa5", "SAN5.POSTa5", "SAN5.POSTa1",
        "SAN1.POSTb1", "SAN1.POSTb5", "SAN5.POSTb5", "SAN5.POSTb1",

        // SBN - Step Below Note variants
        "SBN1", "SBN5",
        "SBN1.PREb1", "SBN1.PREb5", "SBN5.PREb5", "SBN5.PREb1",
        "SBN1.PREa1", "SBN1.PREa5", "SBN5.PREa5", "SBN5.PREa1",
        "SBN1.POSTb1", "SBN1.POSTb5", "SBN5.POSTb5", "SBN5.POSTb1",
        "SBN1.POSTa1", "SBN1.POSTa5", "SBN5.POSTa5", "SBN5.POSTa1",

        // LURP - Long Up Repeat Previous variants
        "LURP",
        "LURP1.PREb1", "LURP1.PREb5", "LURP5.PREb5", "LURP5.PREb1",
        "LURP1.PREa1", "LURP1.PREa5", "LURP5.PREa5", "LURP5.PREa1",
        "LURP1.POSTb1", "LURP1.POSTb5", "LURP5.POSTb5", "LURP5.POSTb1",
        "LURP1.POSTa1", "LURP1.POSTa5", "LURP5.POSTa5", "LURP5.POSTa1",

        // LDRP - Long Down Repeat Previous variants
        "LDRP",
        "LDRP1.PREb1", "LDRP1.PREb5", "LDRP5.PREb5", "LDRP5.PREb1",
        "LDRP1.PREa1", "LDRP1.PREa5", "LDRP5.PREa5", "LDRP5.PREa1",
        "LDRP1.POSTb1", "LDRP1.POSTb5", "LDRP5.POSTb5", "LDRP5.POSTb1",
        "LDRP1.POSTa1", "LDRP1.POSTa5", "LDRP5.POSTa5", "LDRP5.POSTa1",

        // LUSA - Long Up Step Above variants
        "LUSA1", "LUSA5",
        "LUSA1.PREb1", "LUSA1.PREb5", "LUSA5.PREb5", "LUSA5.PREb1",
        "LUSA1.PREa1", "LUSA1.PREa5", "LUSA5.PREa5", "LUSA5.PREa1",
        "LUSA1.POSTb1", "LUSA1.POSTb5", "LUSA5.POSTb5", "LUSA5.POSTb1",
        "LUSA1.POSTa1", "LUSA1.POSTa5", "LUSA5.POSTa5", "LUSA5.POSTa1",

        // LUSB - Long Up Step Below variants
        "LUSB1", "LUSB5",
        "LUSB1.PREb1", "LUSB1.PREb5", "LUSB5.PREb5", "LUSB5.PREb1",
        "LUSB1.PREa1", "LUSB1.PREa5", "LUSB5.PREa5", "LUSB5.PREa1",
        "LUSB1.POSTb1", "LUSB1.POSTb5", "LUSB5.POSTb5", "LUSB5.POSTb1",
        "LUSB1.POSTa1", "LUSB1.POSTa5", "LUSB5.POSTa5", "LUSB5.POSTa1",

        // LDSA - Long Down Step Above variants
        "LDSA1", "LDSA5",
        "LDSA1.PREb1", "LDSA1.PREb5", "LDSA5.PREb5", "LDSA5.PREb1",
        "LDSA1.PREa1", "LDSA1.PREa5", "LDSA5.PREa5", "LDSA5.PREa1",
        "LDSA1.POSTb1", "LDSA1.POSTb5", "LDSA5.POSTb5", "LDSA5.POSTb1",
        "LDSA1.POSTa1", "LDSA1.POSTa5", "LDSA5.POSTa5", "LDSA5.POSTa1",

        // LDSB - Long Down Step Below variants
        "LDSB1", "LDSB5",
        "LDSB1.PREb1", "LDSB1.PREb5", "LDSB5.PREb5", "LDSB5.PREb1",
        "LDSB1.PREa1", "LDSB1.PREa5", "LDSB5.PREa5", "LDSB5.PREa1",
        "LDSB1.POSTb1", "LDSB1.POSTb5", "LDSB5.POSTb5", "LDSB5.POSTb1",
        "LDSB1.POSTa1", "LDSB1.POSTa5", "LDSB5.POSTa5", "LDSB5.POSTa1"
    };

    // Randomly select a variant
    selectedVariant = variants[rand() % variants.size()];

    // Use the comprehensive implementation with DUPLE meter by default
    return applyAppoggiatura(pi, durPi, DUPLE, selectedVariant);
}
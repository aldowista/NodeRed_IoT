
/*
// Generate IV (once)
void aes_init() {
  //Serial.println("gen_iv()");
  aesLib.gen_iv(aes_iv);
  //Serial.println("encrypt_impl()");
  //Serial.println(encrypt_impl(strdup(plaintext.c_str()), aes_iv));
}

String encrypt_impl(char * msg) {
  byte iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; 
  int msgLen = strlen(msg);
  char encrypted[ msgLen] = {0};
  aesLib.encrypt64((const byte*)msg, msgLen, encrypted, aes_key, sizeof(aes_key), iv);
  //Serial.println("Encryption Result: "+String(encrypted));
  return String(encrypted);
}

String decrypt_impl(char * msg) {
  byte iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; 
  int msgLen = strlen(msg);
  char decrypted[msgLen] = {0}; // half may be enough
  aesLib.decrypt64(msg, msgLen, (byte*)decrypted, aes_key, sizeof(aes_key), iv);
  return String(decrypted);
}
*/


void setupAES(){
  aes128 = AES128();
  aes128.setKey(aes_key,sizeof(aes_key));
}

// Function to perform PKCS#7 padding
void pkcs7Padding(uint8_t *data, size_t &len, size_t maxLen) {
    size_t paddingSize = 16 - (len % 16);
    if (len + paddingSize > maxLen) {
        // Handle error: not enough space for padding
        return;
    }
    for (size_t i = 0; i < paddingSize; ++i) {
        data[len + i] = static_cast<uint8_t>(paddingSize);
    }
    len += paddingSize;
}

// Function to encrypt and encode data in base64
String encryptAndEncodeBase64(const uint8_t *input, size_t len) {
    const size_t maxDataSize = 256;
    uint8_t data[maxDataSize];
    if (len > maxDataSize) {
        return String("");  // Input too large
    }
    memcpy(data, input, len);

    // Apply PKCS#7 padding
    pkcs7Padding(data, len, maxDataSize);

    // Encrypt each block and concatenate
    size_t encryptedDataSize = len; // len has been updated to include padding
    uint8_t encryptedData[encryptedDataSize]; // Ensure this buffer is large enough
    for (size_t i = 0; i < len; i += 16) {
        aes128.encryptBlock(encryptedData + i, &data[i]);
    }

    // Encode the entire concatenated encrypted data
    return base64_encode(encryptedData, encryptedDataSize);
}


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

String hash_impl(char * msg){
  int msgLen = strlen(msg);
  SHA256 hasher = SHA256();
  hasher.update(msg,msgLen);

  int hashlen = hasher.hashSize();
  char hash[hashlen] = {0};
  hasher.finalize(hash,hashlen);

  int encodedLen = base64_enc_len(hashlen);
  char encodedHash[encodedLen];
  base64_encode(encodedHash, hash, hashlen);

  return String(encodedHash);
}

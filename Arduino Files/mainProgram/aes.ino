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
  char encrypted[2 * msgLen] = {0};
  aesLib.encrypt64((const byte*)msg, msgLen, encrypted, aes_key, sizeof(aes_key), iv);
  return String(encrypted);
}

String decrypt_impl(char * msg) {
  byte iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; 
  int msgLen = strlen(msg);
  char decrypted[msgLen] = {0}; // half may be enough
  aesLib.decrypt64(msg, msgLen, (byte*)decrypted, aes_key, sizeof(aes_key), iv);
  return String(decrypted);
}

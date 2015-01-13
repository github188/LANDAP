
#ifndef _BASE64_H_
#define _BASE64_H_

#include "ddp_general.h"

#ifdef __cplusplus
extern "C" {
#endif 

/* base64_calculate_encrypted_length
 *   fucntion to calculate length of encoded string
 *
 *   strIn : input string to be encoded
 *
 *   return : length of encoded string
 */
INT4 base64_calculate_encrypted_length(INT1* strIn);
/* base64_calculate_decrypted_length
 *   function to calculate length of decoded string
 *
 *   strIn : string is going to be decoded
 *
 *   return : length of decoded string
 */
INT4 base64_calculate_decrypted_length(INT1* strIn);

/* base64_encode
 *   function to encode a string according to base 64 algorithm
 *
 *   strIn : input string to be encoded
 *   strOut : buffer for output string
 *   iOutBufSize : size of strOut buffer
 *
 *   return : actual length of output string
 */
INT4 base64_encode(INT1* strIn, INT1* strOut, INT4 iOutBufSize);
/* base64_decode
 *   function to decode a string according to base 64 algorithm
 *
 *   strIn : input string to be decoded
 *   strOut : buffer for decoded string
 *   iOutBufSize : size for strOut buffer
 *
 *   return : actual length of output string
 */
INT4 base64_decode(INT1* strIn, INT1* strOut, INT4 iOutBufSize);

#ifdef __cplusplus
}
#endif

#endif /* _BASE64_H_ */


def init_encipher(key):
    k = [0]*68
    key = [ord(i) for i in key]
    j = 0
    while j < 60:
        for i in range(15):
            k[j] = key[i]
            j = j + 1
    return k
    
def init_decipher(key):
    k = [0]*68
    key = [ord(i) for i in key]
    i = 11
    j = 0
    q = 0
    
    while True:
        k[q] = key[i]
        q = q + 1
        i = ((i+1) % 15 )
        k[q] = key[i]
        q = q + 1
        i = ((i+1) % 15 )
        k[q] = key[i]
        q = q + 1
        i = ((i+1) % 15 )

        k[q] = key[i]
        q = q + 1
        i = (i+9) % 15;
        if i == 12: 
            break

        k[q] = key[i]
        i = i + 1
        q = q + 1
        k[q] = key[i]
        i = i + 1
        q = q + 1
        k[q] = key[i]
        q = q + 1
        i = (i+9) % 15

    return k

    
def cipher_8(k,S):
    b = [ord(i) for i in S]
    q = 0

    rotor = [\
        32,137,239,188,102,125,221, 72,212, 68, 81, 37, 86,237,147,149,
        70,229, 17,124,115,207, 33, 20,122,143, 25,215, 51,183,138,142,
       146,211,110,173,  1,228,189, 14,103, 78,162, 36,253,167,116,255,
       158, 45,185, 50, 98,168,250,235, 54,141,195,247,240, 63,148,  2,
       224,169,214,180, 62, 22,117,108, 19,172,161,159,160, 47, 43,171,
       194,175,178, 56,196,112, 23,220, 89, 21,164,130,157,  8, 85,251,
       216, 44, 94,179,226, 38, 90,119, 40,202, 34,206, 35, 69,231,246,
        29,109, 74, 71,176,  6, 60,145, 65, 13, 77,151, 12,127, 95,199,
        57,101,  5,232,150,210,129, 24,181, 10,121,187, 48,193,139,252,
       219, 64, 88,233, 96,128, 80, 53,191,144,218, 11,106,132,155,104,
        91,136, 31, 42,243, 66,126,135, 30, 26, 87,186,182,154,242,123,
        82,166,208, 39,152,190,113,205,114,105,225, 84, 73,163, 99,111,
       204, 61,200,217,170, 15,198, 28,192,254,134,234,222,  7,236,248,
       201, 41,177,156, 92,131, 67,249,245,184,203,  9,241,  0, 27, 46,
       133,174, 75, 18, 93,209,100,120, 76,213, 16, 83,  4,107,140, 52,
        58, 55,  3,244, 97,197,238,227,118, 49, 79,230,223,165,153, 59
      ]

    for i in range(8):
        
        b[4] = b[4] ^ rotor[b[0] ^ k[q]]; q += 1
        b[5] = b[5] ^ rotor[b[1] ^ k[q]]; q += 1
        b[6] = b[6] ^ rotor[b[2] ^ k[q]]; q += 1
        b[7] = b[7] ^ rotor[b[3] ^ k[q]]; q += 1

        b[1] = b[1] ^ rotor[b[4] ^ k[q]]; q += 1
        b[2] = b[2] ^ rotor[b[4] ^ b[5]];
        b[3] = b[3] ^ rotor[b[6] ^ k[q]]; q += 1
        b[0] = b[0] ^ rotor[b[7] ^ k[q]]; q += 1
    
    b[4] = b[4] ^ rotor[b[0] ^ k[q]]; q += 1
    b[5] = b[5] ^ rotor[b[1] ^ k[q]]; q += 1
    b[6] = b[6] ^ rotor[b[2] ^ k[q]]; q += 1
    b[7] = b[7] ^ rotor[b[3] ^ k[q]]; q += 1

    return ''.join([chr(i) for i in b]) 

def encrypt_S(pwd,S):
    while len(pwd) < 15: pwd = pwd + "^"
    k = init_encipher(pwd)
    while len(S) % 8: S = S + "\0"
    r = ""
    while S:
        r = r + cipher_8(k,S[:8])
        S = S[8:]
    return r

def decrypt_S(pwd,S):
    while len(pwd) < 15: pwd = pwd + "^"
    k = init_decipher(pwd)
    r = ""
    while S:
        r = r + cipher_8(k,S[:8])
        S = S[8:]
    return r
    
if __name__ == '__main__':
    
    T = "mama myla ramu papa delal stul vse baly dovolny tolko stul ne dddd......"
    pwd = "123456789asdfg"
    
    S = encrypt_S(pwd,T)
    print repr(S)
    S = decrypt_S(pwd,S)
    print repr(S)

    
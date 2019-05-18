#ifndef RPTX_AES_H
#define RPTX_AES_H

#include <array>

//TODO get rid
#define getSBoxValue(num) (sbox[(num)])

namespace pug::crypto {

    /**
     * R as the round number - round keys needed:
     * + 11 round keys for AES-128
     * + 13 keys for AES-192
     * + 15 keys for AES-256
     */
    //constexpr size_t R128 = 11;
    //constexpr size_t R192 = 13;
    constexpr size_t R256 = 15;

    /**
     * N as the length of the key in 32-bit words:
     * + 4 words AES-128
     * + 6 words AES-192
     * + 8 words AES-256
     */
    //constexpr size_t N128 = 4;
    //constexpr size_t N192 = 6;
    constexpr size_t N256 = 8;

    /**
     * @brief AES block cipher algorithm implementation available choices are AES128, AES192, AES256.
     * The implementation is verified against the test vectors in:
     * National Institute of Standards and Technology Special Publication 800-38A 2001 ED
     * @tparam R number of round keys needed (AES-256 default 15)
     * @tparam N length of the key in 32-bit words (AES-256 default 8)
     * @tparam T 8-bit type (default uint8_t) manipulating 8-bit bytes obviates the need to handle endianness across platforms.
     */
    template<size_t R = R256, size_t N = N256, typename T = uint8_t>
    class aes {

        /**
         * 128-bit block size.
         * Until the announcement of NIST's AES contest, the majority of block ciphers followed the example of the DES in using a block size of 64 bits (8 bytes).
         * However, the birthday paradox tells us that after accumulating a number of blocks equal to the square root of the total number possible,
         * there will be an approximately 50% chance of two or more being the same, which would start to leak information about the message contents.
         * Consequently, AES candidates were required to support a block length of 128 bits (16 bytes).
         * This should be acceptable for up to 2^64 × 16 B = 256 exabytes of data - which should suffice for quite a few years to come.
         * The winner of the AES contest, Rijndael, supports block and key sizes of 128, 192, and 256 bits, but in AES the block size is always 128 bits.
         */
        constexpr static size_t BLOCK_SIZE = 16;

        /**
          * K as the length of the key in 8-bit bytes:
          * + 16 bytes AES-128
          * + 24 bytes AES-192
          * + 32 bytes AES-256
          * i.e N x 4
          */
        constexpr static size_t K = N * 4;

        /**
         * XK as the length of the expanded key in 8-bit bytes.
         */
        constexpr static size_t XK = R * N * 2;

        /**
         * O as the offset into the rcon array.
         * i.e. ((R - 1) x N)
         */
        constexpr static size_t O = ((R - 1) * N);

        using key_t = std::array<T, K>;
        using expanded_key_t = std::array<T, XK>;

    public:

        using block_t = std::array<T, BLOCK_SIZE>;
        using state_t = T[4][4];

        template<class Sequence>
        explicit aes(Sequence&& seq) noexcept;

        aes(aes const&) = delete;

        aes& operator=(aes const&) = delete;

        ~aes() = default;

        /**
         * @brief Encrypt a 16 byte block of plaintext using the session key material
         * @param p the plaintext(state_t*)buf
         * @return 16 byte block of ciphertext
         */
        block_t encrypt(state_t* state) {//TODO const block_t&
            uint8_t round = 0;
            // Add the First round key to the state before starting the rounds.
            //AddRoundKey(round, state);
            add_round_key(round, (uint8_t*)state);
            // The first R - 1 rounds are identical.
            // These R - 1 rounds are executed in the loop below.
            auto i = R - 1;
            while(--i) {
                //SubBytes(state);
                sub_bytes((uint8_t*)state);
                //ShiftRows(state); // Rijndael diffusion
                shift_rows((uint8_t*)state);
                //MixColumns(state);
                mix_columns((uint8_t*)state); // Rijndael diffusion
                //AddRoundKey(round, state);
                add_round_key(round, (uint8_t*)state);
            }
            // The last round is given below.
            // The MixColumns function is not here in the last round.
            //SubBytes(state);
            sub_bytes((uint8_t*)state);
            //ShiftRows(state);
            shift_rows((uint8_t*)state);
            //AddRoundKey(round, state);
            add_round_key(round, (uint8_t*)state);
        }

        /**
         * @brief Decrypt a 16 byte block of cyphertext using the session key material
         * @param c the cyphertext
         * @return 16 byte block of plaintext
         */
        block_t decrypt(const block_t& c) {

        }

#ifndef NDEBUG
        expanded_key_t& debug_xkey() {
            return xkey;
        }
#endif

    private:

        // This function adds the round key to state.
        // The round key is added to the state by an XOR function.
        void AddRoundKey(uint8_t round, state_t* state) {
            uint8_t i,j;
            for (i = 0; i < 4; ++i)
            {
                for (j = 0; j < 4; ++j)
                {
                    (*state)[i][j] ^= xkey[(round * 4 * 4) + (i * 4) + j];
                }
            }
        }

        void add_round_key(uint8_t& round, uint8_t* state) {
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
            *(state++) ^= xkey[round++];
        }

        // The SubBytes Function Substitutes the values in the
        // state matrix with values in an S-box.
        void SubBytes(state_t* state) {
            uint8_t i, j;
            for (i = 0; i < 4; ++i)
            {
                for (j = 0; j < 4; ++j)
                {
                    (*state)[j][i] = getSBoxValue((*state)[j][i]);
                }
            }
        }

        void sub_bytes(uint8_t* state) {
            *(state + 0 ) = sbox[*(state + 0 )];
            *(state + 1 ) = sbox[*(state + 1 )];
            *(state + 2 ) = sbox[*(state + 2 )];
            *(state + 3 ) = sbox[*(state + 3 )];
            *(state + 4 ) = sbox[*(state + 4 )];
            *(state + 5 ) = sbox[*(state + 5 )];
            *(state + 6 ) = sbox[*(state + 6 )];
            *(state + 7 ) = sbox[*(state + 7 )];
            *(state + 8 ) = sbox[*(state + 8 )];
            *(state + 9 ) = sbox[*(state + 9 )];
            *(state + 10) = sbox[*(state + 10)];
            *(state + 11) = sbox[*(state + 11)];
            *(state + 12) = sbox[*(state + 12)];
            *(state + 13) = sbox[*(state + 13)];
            *(state + 14) = sbox[*(state + 14)];
            *(state + 15) = sbox[*(state + 15)];
        }

        // The ShiftRows() function shifts the rows in the state to the left.
// Each row is shifted with different offset.
// Offset = Row number. So the first row is not shifted.
        void ShiftRows(state_t* state)
        {
            uint8_t temp;

            // Rotate first row 1 columns to left
            temp           = (*state)[0][1];
            (*state)[0][1] = (*state)[1][1];
            (*state)[1][1] = (*state)[2][1];
            (*state)[2][1] = (*state)[3][1];
            (*state)[3][1] = temp;

            // Rotate second row 2 columns to left
            temp           = (*state)[0][2];
            (*state)[0][2] = (*state)[2][2];
            (*state)[2][2] = temp;

            temp           = (*state)[1][2];
            (*state)[1][2] = (*state)[3][2];
            (*state)[3][2] = temp;

            // Rotate third row 3 columns to left
            temp           = (*state)[0][3];
            (*state)[0][3] = (*state)[3][3];
            (*state)[3][3] = (*state)[2][3];
            (*state)[2][3] = (*state)[1][3];
            (*state)[1][3] = temp;
        }

        void shift_rows(uint8_t* state) {
            uint8_t rol;
            // Rotate first row 1 columns to left
            rol = *(state + 1);
            *(state + 1) = *(state + 5);
            *(state + 5) = *(state + 9);
            *(state + 9) = *(state + 13);
            *(state + 13) = rol;
            // Rotate second row 2 columns to left
            rol = *(state + 2);
            *(state + 2) = *(state + 10);
            *(state + 10) = rol;
            rol = *(state + 6);
            *(state + 6) = *(state + 14);
            *(state + 14) = rol;
            // Rotate third row 3 columns to left
            rol = *(state + 3);
            *(state + 3) = *(state + 15);
            *(state + 15) = *(state + 11);
            *(state + 11) = *(state + 7);
            *(state + 7) = rol;
        }

        //multiplied by 2 in Rijndael's Galois field
        uint8_t GF2(uint8_t x) {
            return (x<<1)           //implicitly removes high bit because 8-bit, (so * 0x1b and not 0x11b)
                   ^               //xor
                   (((x>>7) & 1)   // arithmetic right shift, thus shifting in either zeros or ones
                    * 0x1b);        // Rijndael's Galois field
        }


        // MixColumns function mixes the columns of the state matrix
        void MixColumns(state_t* state) {
            uint8_t i;
            uint8_t Tmp, Tm, t;
            for (i = 0; i < 4; ++i)
            {
                t   = (*state)[i][0];
                Tmp = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3] ;
                Tm  = (*state)[i][0] ^ (*state)[i][1] ; Tm = GF2(Tm);  (*state)[i][0] ^= Tm ^ Tmp ;
                Tm  = (*state)[i][1] ^ (*state)[i][2] ; Tm = GF2(Tm);  (*state)[i][1] ^= Tm ^ Tmp ;
                Tm  = (*state)[i][2] ^ (*state)[i][3] ; Tm = GF2(Tm);  (*state)[i][2] ^= Tm ^ Tmp ;
                Tm  = (*state)[i][3] ^ t ;              Tm = GF2(Tm);  (*state)[i][3] ^= Tm ^ Tmp ;
            }
        }

        /**
         * @brief consider 16 byte block as 4x4 matrix and mix columns as per Rijndael algorithm.
         * The operation consists in the modular multiplication of two four-term polynomials, whose coefficients are
         * elements of _GF(2^8)_. The modulo used for this operation is _x^4+1_.
         * @param state
         */
        void mix_columns(uint8_t* state) {
            uint8_t c, b, a;

            a   = *(state + 0);
            c = *(state + 0) ^ *(state + 1) ^ *(state + 2) ^ *(state + 3);
            b  = *(state + 0) ^ *(state + 1);
            b = GF2(b);
            *(state + 0) ^= b ^ c;
            b  = *(state + 1) ^ *(state + 2);
            b = GF2(b);
            *(state + 1) ^= b ^ c;
            b  = *(state + 2) ^ *(state + 3);
            b = GF2(b);
            *(state + 2) ^= b ^ c;
            b  = *(state + 3) ^ a ;
            b = GF2(b);
            *(state + 3) ^= b ^ c;

            a   = *(state + 4);
            c = *(state+ 4) ^ *(state+ 5) ^ *(state+ 6) ^ *(state + 7);
            b  = *(state + 4) ^ *(state + 5);
            b = GF2(b);
            *(state + 4) ^= b ^ c;
            b  = *(state + 5) ^ *(state + 6);
            b = GF2(b);
            *(state + 5) ^= b ^ c;
            b  = *(state + 6) ^ *(state + 7);
            b = GF2(b);
            *(state + 6) ^= b ^ c;
            b  = *(state + 7) ^ a ;
            b = GF2(b);
            *(state + 7) ^= b ^ c;

            a   = *(state + 8);
            c = *(state + 8) ^ *(state + 9) ^ *(state + 10) ^ *(state + 11);
            b  = *(state + 8) ^ *(state + 9);
            b = GF2(b);
            *(state + 8) ^= b ^ c;
            b  = *(state + 9) ^ *(state + 10);
            b = GF2(b);
            *(state + 9) ^= b ^ c;
            b  = *(state + 10) ^ *(state + 11);
            b = GF2(b);
            *(state + 10) ^= b ^ c;
            b  = *(state + 11) ^ a ;
            b = GF2(b);
            *(state + 11) ^= b ^ c;

            a   = *(state + 12);
            c = *(state + 12) ^ *(state + 13) ^ *(state + 14) ^ *(state + 15);
            b  = *(state + 12) ^ *(state + 13);
            b = GF2(b);
            *(state + 12) ^= b ^ c;
            b  = *(state + 13) ^ *(state + 14);
            b = GF2(b);
            *(state + 13) ^= b ^ c;
            b  = *(state + 14) ^ *(state + 15);
            b = GF2(b);
            *(state + 14) ^= b ^ c;
            b  = *(state + 15) ^ a ;
            b = GF2(b);
            *(state + 15) ^= b ^ c;
        }
        
        /**
         * @brief This function produces Nb(Nr+1) round keys. 
         * The round keys are used in each round to decrypt the states.
         * @param key 
         */
        void make_expanded_key(const key_t& key);

        /**
         * S-box (substitution-box) basic component of symmetric key algorithms which performs substitution. 
         * Used to obscure the relationship between the key and the ciphertext — Shannon's property of confusion.
         * Takes some number of input bits _m_ and transforms them into some number of output bits _n_.
         * An m×n S-box can be implemented as a lookup table with 2m words of n bits each. 
         * Fixed tables are normally used, as in the AES, but in some ciphers the tables are generated dynamically from the key 
         * e.g. the _Blowfish_ and the _Twofish_ encryption algorithms.
         */
        static constexpr uint8_t sbox[256] = {
                //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
                0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
                0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
                0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
                0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
                0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
                0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
                0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
                0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
                0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
                0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
                0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
                0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
                0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
                0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
                0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
                0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

        /**
         * The round constant word array, Rcon[i], contains the values given by x^(i-1) being powers of x in the Galois field GF(2^8)
         */
        static constexpr uint8_t Rcon[11] = { 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };

        expanded_key_t xkey;

    };

    template<size_t R, size_t N, typename T>
    template<class Sequence>
    aes<R, N, T>::aes(Sequence &&seq) noexcept {
        key_t key;
        auto it = std::begin(seq);
        for(int i = 0; i < key.size(); ++i) {
            key[i] = *it++;
        }
        make_expanded_key(key);
    }

    template<size_t R, size_t N, typename T>
    void aes<R, N, T>::make_expanded_key(const aes::key_t &key) {
        unsigned i, j, k;
        unsigned Nk = key.size() / 4;
        std::array<T, 4> w{}; // 32-bit Rijndael word used for the column/row operations
        // The first round key is the key itself.
        for (i = 0; i < Nk; ++i) {
            xkey[(i * 4) + 0] =key[(i * 4) + 0];
            xkey[(i * 4) + 1] =key[(i * 4) + 1];
            xkey[(i * 4) + 2] =key[(i * 4) + 2];
            xkey[(i * 4) + 3] =key[(i * 4) + 3];
        }
        // All other round keys are found from the previous round keys.
        for (i = Nk; i < xkey.size() / 4; ++i) {
            k = (i - 1) * 4;
            w[0]=xkey[k + 0];
            w[1]=xkey[k + 1];
            w[2]=xkey[k + 2];
            w[3]=xkey[k + 3];

            if (i % Nk == 0) {
                //shift the 4 bytes in a word to the left once [a0,a1,a2,a3] becomes [a1,a2,a3,a0]
                const T rol = w[0];
                w[0] = w[1];
                w[1] = w[2];
                w[2] = w[3];
                w[3] = rol;
                //sbox mixing
                w[0] = sbox[w[0]];
                w[1] = sbox[w[1]];
                w[2] = sbox[w[2]];
                w[3] = sbox[w[3]];
                //Galois Field mix xor round constant
                w[0] = w[0] ^ Rcon[i/Nk];
            }
#if (R == R256) //extension for AES-256
            if (i % Nk == 4) {
                w[0] = sbox[w[0]];
                w[1] = sbox[w[1]];
                w[2] = sbox[w[2]];
                w[3] = sbox[w[3]];
            }
#endif
            //use the mixed word _w_ to xor expand preceding key word into subsequent one
            j = i * 4;
            k=(i - Nk) * 4;
            xkey[j + 0] = xkey[k + 0] ^ w[0];
            xkey[j + 1] = xkey[k + 1] ^ w[1];
            xkey[j + 2] = xkey[k + 2] ^ w[2];
            xkey[j + 3] = xkey[k + 3] ^ w[3];
        }
    }

}

#endif //RPTX_AES_H
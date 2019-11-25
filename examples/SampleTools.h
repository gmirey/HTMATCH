/* -----------------------------------
 * HTMATCH
 * SampleTools.h
 * -----------------------------------
 * Defines a few tools outside of VanillaHTM per se.
 * Those may be handy to quickly build test applications
 *
 * Copyright 2019, Guillaume Mirey
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _HTMATCH_SAMPLE_TOOLS_H
#define _HTMATCH_SAMPLE_TOOLS_H

#include "tools/system.h"
#include <vector>
#include <algorithm>

namespace HTMATCH {

    class ColorTools {

    public:

        // shows colouring adapted to distinguish low int values, but able to display range up to 1024
        //   black at 0, to blue at 8, to cyan at 25, to green at 42,
        //   to yellow at 93, to red at 144, to magenta at 399, to white (saturating) at 1164
        template<typename ValueType>
        static void color1K(ValueType value, uint8& outRed, uint8& outGreen, uint8& outBlue) {
            outRed = 0u;
            outGreen = 0u;
            outBlue = 0u;
            if (value > 0) {
                if (value <= 8)
                    outBlue = uint8(ValueType(95) + value * 20);    // 115, 135, 155, 175, 195, 215, 235, 255
                else if (value <= ValueType(25)) {
                    outBlue = 255u;
                    outGreen = uint8((value-ValueType(8)) * 15);
                } else if (value <= ValueType(42)) {
                    outGreen = 255u;
                    outBlue = uint8(ValueType(255) - (value-ValueType(25)) * 15);
                } else if (value <= ValueType(93)) {
                    outGreen = 255u;
                    outRed = uint8((value-ValueType(42)) * 5);
                } else if (value <= ValueType(144)) {
                    outRed = 255u;
                    outGreen = uint8(ValueType(255) - (value-ValueType(93)) * 5);
                } else if (value <= ValueType(399)) {
                    outRed = 255u;
                    outBlue = uint8(value - ValueType(144));
                } else {
                    outRed = 255u;
                    outBlue = 255u;
                    outGreen = uint8(std::max(ValueType((value-ValueType(399)) / 3), ValueType(255)));
                }
            }
        }
        ; // template termination

        static void color256K(uint32 value, uint8& outRed, uint8& outGreen, uint8& outBlue) {
            // calls 'color1K' with 256 -> 1.0f    (boosted values map times 256 as "default")
            color1K<float>(float(value) * 0.00390625f, outRed, outGreen, outBlue);
        }
        static void colorBoosting(uint16 value, uint8& outRed, uint8& outGreen, uint8& outBlue) {
            // calls 'color1K' with 256 -> 32.0f   (256 is "default" no boost => shown inbetween cyan and green)
            color1K<float>(float(value) * 0.125f, outRed, outGreen, outBlue);                       
        }

        // When uScale32 == 32   : same as colorBoosting    128 -> 16.0 ; 256 -> 32.0 ; 512 -> 64.0
        // When uScale32 == 16   :                          128 -> 24.0 ; 256 -> 32.0 ; 512 -> 48.0
        // When uScale32 == 64   :                          192 -> 16.0 ; 256 -> 32.0 ; 384 -> 64.0
        template<unsigned int uScale32>
        static void colorScaledBoosting(uint16 value, uint8& outRed, uint8& outGreen, uint8& outBlue) {
            float fMinus256 = float(value) - 256.0f;
            color1K<float>(std::max(0.0f, 32.0f + fMinus256 * (float(uScale32) * 0.00390625f)), outRed, outGreen, outBlue);                       
        }
        ; // template termination

    };

    // Currently designed to be somewhat "clear" when viewed col major on four 64x32 bitmaps
    class FixedDigitEncoder {
    public:
        FixedDigitEncoder() {
            // digits (codes 0..9)
            for (uint8 uDigit = 0u; uDigit <= 9u; uDigit++) {
                _initInputCode(uDigit);
                _initDigit(uDigit);
            }
            // min and maj letters (codes 10..35 for lower case and 36..61 for upper case)
            for (uint8 uLetter = 0u; uLetter < 26u; uLetter++) {
                for (uint8 uUpper = 0u; uUpper < 2u; uUpper++) {
                    uint8 uCode = (uLetter + uUpper*26u) + 10u;
                    _initInputCode(uCode);
                    _initLetter(uCode, uLetter, uUpper);
                }
            }
            // dash (code 62)
            _initInputCode(62u);
            _initDash();
            // space (code 63)
            _initInputCode(63u);
            _initSpace();
        }

        const std::vector<uint16>& getInputVectorEncodingDigitCode(u8fast uDigitCode6b) const {
            return _tInputsPerChar[std::min(uDigitCode6b, u8fast(64u))];
        }

        static const uint8 getDigitCode6bFromChar(unsigned char c) {
            if (c >= '0' && c <= '9')
                return uint8(c - '0');
            else if (c >= 'a' && c <= 'z' )
                return uint8(10u + (c - 'a'));
            else if (c >= 'A' && c <= 'Z' )
                return uint8(36u + (c - 'A'));
            else if (c == '-')
                return 62u;
            else if (c == ' ')
                return 63u;
            else
                return 64u;
        }

    private:
        std::vector<uint16> _tInputsPerChar[65u];   // 0..63 valid ; 64 empty

        void _addVertLineToVec(u8fast uCode, u8fast uPosX, u8fast uPosY, u8fast uPosZ, u8fast uSize) {
            std::vector<uint16>& vecToFill = _tInputsPerChar[uCode];
            uint16 uStartIndex = uint16(uPosZ) * 2048u + uint16(uPosX) * 32u;
            for (u8fast uIndex = uPosY, uEnd = uPosY+uSize; uIndex < uEnd; uIndex++)
                vecToFill.push_back(uStartIndex + uint16(uIndex));
        }

        void _addSquare6x6ToVec(u8fast uCode, u8fast uPosX, u8fast uPosY, u8fast uPosZ) {
            for (u8fast uX = uPosX, uEndX = uPosX+6u; uX < uEndX; uX++)
                _addVertLineToVec(uCode, uX, uPosY, uPosZ, 6u);
        }

        void _addSquare2x2ToVec(u8fast uCode, u8fast uPosX, u8fast uPosY, u8fast uPosZ) {
            for (u8fast uX = uPosX, uEndX = uPosX+2u; uX < uEndX; uX++)
                _addVertLineToVec(uCode, uX, uPosY, uPosZ, 2u);
        }

        void _initInputCode(u8fast uCode) {
            // 6x6 pix square encodings
            // 4x vertically (2x on/off, twice for two rows of 3b each)
            // 3x horizontally (for each of the 3 bits)
            for (u8fast uBit = 0u; uBit < 6u; uBit++) {  // 6 bits in a 0..63 value
                u8fast uPosX = (uBit % 3u) * 8u;
                u8fast uPosY = (uBit / 3u) * 16u; 
                if (uCode & (1u << uBit)) {
                    uPosY += 8u;
                }
                _addSquare6x6ToVec(uCode, uPosX, uPosY, 0u);
                _addSquare2x2ToVec(uCode, uPosX, uPosY, 2u);
            }
        }

        void _initDigit(u8fast uDigit) {
            for (u8fast uX = 0u; uX < 13u; uX++) {
                u8fast uPosX = uX + 24u + uDigit*3u;
                _addVertLineToVec(uDigit, uPosX, 0u, 0u, 10u);
                _addVertLineToVec(uDigit, uPosX, 0u, 2u, 1u);
            }
            for (u8fast uX = 0u; uX < 5u; uX++) {
                u8fast uPosX = uX + uDigit*3u;
                _addVertLineToVec(uDigit, uPosX, 8u, 1u, 16u);
                _addVertLineToVec(uDigit, uPosX, 8u, 3u, 2u);
            }
            _addSquare6x6ToVec(uDigit, 32u, 16u, 1u);
            _addSquare6x6ToVec(uDigit, 40u, 16u, 1u);
            _addSquare2x2ToVec(uDigit, 32u, 16u, 3u);
            _addSquare2x2ToVec(uDigit, 40u, 16u, 3u);
        }

        void _initLetter(u8fast uCode, u8fast uLetter, u8fast uUpper) {
            for (u8fast uX = 0u; uX < 15u; uX++) {
                u8fast uPosX = uX + 24u + uLetter;
                _addVertLineToVec(uCode, uPosX, 10u + uUpper*6u, 0u, 10u);
                _addVertLineToVec(uCode, uPosX, 10u + uUpper, 2u, 2u);
            }
            for (u8fast uX = 0u; uX < 14u; uX++) {
                u8fast uPosX = uX + uLetter*2u;
                _addVertLineToVec(uCode, uPosX, uUpper*24u, 1u, 8u);
                _addVertLineToVec(uCode, uPosX, uUpper*24u, 3u, 1u);
            }
            _addSquare6x6ToVec(uCode, 32u+uUpper*8u, 8u, 1u);
            _addSquare6x6ToVec(uCode, 40u+uUpper*8u, 8u, 1u);
            _addSquare2x2ToVec(uCode, 32u+uUpper*8u, 8u, 3u);
            _addSquare2x2ToVec(uCode, 40u+uUpper*8u, 8u, 3u);
        }

        void _initDash() {
            for (u8fast uX = 0u; uX < 20u; uX++) {
                u8fast uPosX = uX + 24u;
                _addVertLineToVec(62u, uPosX, 26u, 0u, 7u);
                _addVertLineToVec(62u, uPosX, 26u, 2u, 1u);
            }
            _addSquare6x6ToVec(62, 48u, 16u, 1u);
            _addSquare2x2ToVec(62, 48u, 16u, 3u);
        }

        void _initSpace() {
            for (u8fast uX = 0u; uX < 20u; uX++) {
                u8fast uPosX = uX + 44u;
                _addVertLineToVec(63u, uPosX, 26u, 0u, 7u);
                _addVertLineToVec(63u, uPosX, 26u, 2u, 1u);
            }
            _addSquare6x6ToVec(62, 48u, 16u, 1u);
            _addSquare2x2ToVec(62, 48u, 16u, 3u);
        }
    };

} // namespace HTMATCH

#endif // _HTMATCH_SAMPLE_TOOLS_H


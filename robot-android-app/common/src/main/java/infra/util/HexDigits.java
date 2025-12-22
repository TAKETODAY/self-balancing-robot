/*
 * Copyright 2017 - 2025 the original author or authors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see [https://www.gnu.org/licenses/]
 */

package infra.util;

/**
 * Digits provides a fast methodology for converting integers and longs to
 * hexadecimal digits ASCII strings.
 */
final class HexDigits {
  /**
   * Each element of the array represents the ascii encoded
   * hex relative to its index, for example:<p>
   * <pre>
   *       0 -> '00' -> '0' | ('0' << 8) -> 0x3030
   *       1 -> '01' -> '0' | ('1' << 8) -> 0x3130
   *       2 -> '02' -> '0' | ('2' << 8) -> 0x3230
   *
   *     ...
   *
   *      10 -> '0a' -> '0' | ('a' << 8) -> 0x6130
   *      11 -> '0b' -> '0' | ('b' << 8) -> 0x6230
   *      12 -> '0c' -> '0' | ('b' << 8) -> 0x6330
   *
   *     ...
   *
   *      26 -> '1a' -> '1' | ('a' << 8) -> 0x6131
   *      27 -> '1b' -> '1' | ('b' << 8) -> 0x6231
   *      28 -> '1c' -> '1' | ('c' << 8) -> 0x6331
   *
   *     ...
   *
   *     253 -> 'fd' -> 'f' | ('d' << 8) -> 0x6466
   *     254 -> 'fe' -> 'f' | ('e' << 8) -> 0x6566
   *     255 -> 'ff' -> 'f' | ('f' << 8) -> 0x6666
   * </pre>
   */
  private static final short[] DIGITS;

  static {
    short[] digits = new short[16 * 16];

    for (int i = 0; i < 16; i++) {
      short lo = (short) (i < 10 ? i + '0' : i - 10 + 'a');

      for (int j = 0; j < 16; j++) {
        short hi = (short) ((j < 10 ? j + '0' : j - 10 + 'a') << 8);
        digits[(i << 4) + j] = (short) (hi | lo);
      }
    }

    DIGITS = digits;
  }

  /**
   * Constructor.
   */
  private HexDigits() {
  }

  /**
   * For values from 0 to 255 return a short encoding a pair of hex ASCII-encoded digit characters in little-endian
   *
   * @param i value to convert
   * @param ucase true uppper case, false lower case
   * @return a short encoding a pair of hex ASCII-encoded digit characters
   */
  public static short digitPair(int i, boolean ucase) {
    /*
     * 0b0100_0000_0100_0000 is a selector that selects letters (1 << 6),
     * uppercase or not, and shifting it right by 1 bit incidentally
     * becomes a bit offset between cases (1 << 5).
     *
     *  ([0-9] & 0b100_0000) >> 1 => 0
     *  ([a-f] & 0b100_0000) >> 1 => 32
     *
     *  [0-9] -  0 => [0-9]
     *  [a-f] - 32 => [A-F]
     */
    short v = DIGITS[i & 0xff];
    return ucase
            ? (short) (v - ((v & 0b0100_0000_0100_0000) >> 1))
            : v;
  }
}

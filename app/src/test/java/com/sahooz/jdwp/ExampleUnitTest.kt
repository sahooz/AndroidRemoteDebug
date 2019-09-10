package com.sahooz.jdwp

import org.junit.Test

import org.junit.Assert.*

/**
 * Example local unit test, which will execute on the development machine (host).
 *
 * See [testing documentation](http://d.android.com/tools/testing).
 */
class ExampleUnitTest {
    @Test
    fun addition_isCorrect() {
        assertEquals(4, 2 + 2)
    }

    @Test
    fun testStr() {
        println(String(Hex.decodeHex("416e64726f69642052756e74696d6520322e312e30".toCharArray())))
        println(String(Hex.decodeHex("312e362e30".toCharArray())))
        println(String(Hex.decodeHex("44616c76696b".toCharArray())))
    }
}

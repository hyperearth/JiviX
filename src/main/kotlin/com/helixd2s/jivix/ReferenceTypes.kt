package com.helixd2s.jivix

import java.nio.IntBuffer
import java.nio.LongBuffer

// TODO: Operable Number Objects for Kotlin
class ReferenceTypes {

    companion object;

    open class ULongRef {
        lateinit var value: LongBuffer
    }

    open class LongRef {
        lateinit var value: LongBuffer
    }

    open class UIntRef {
        lateinit var value: IntBuffer
    }

    open class IntRef {
        lateinit var value: IntBuffer
    }

}

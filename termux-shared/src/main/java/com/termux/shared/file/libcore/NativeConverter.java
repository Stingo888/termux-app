package com.termux.shared.file.libcore;

public class NativeConverter {
    public static native Charset charsetForName(String charsetName);
    static { System.loadLibrary("nativeconverter"); }
}

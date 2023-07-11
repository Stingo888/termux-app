package com.termux.shared.file.libcore;
import com.termux.shared.nio.charset.Charset;
public class NativeConverter {
    public static native Charset charsetForName(String charsetName);
    static { System.loadLibrary("nativeconverter"); }
}

package com.termux.shared.file.libcore;

public class PosixBuild {

    public static final int PROP_NAME_MAX = 31;
    public static final int PROP_VALUE_MAX = 91;

    private static native String nativeGet(String key, String def);
    static { System.loadLibrary("posix"); }

    public static final String CPU_ABI = getString("ro.product.cpu.abi");

    public static String get(String key, String def) {
        if (key.length() > PROP_NAME_MAX) {
            throw new IllegalArgumentException("key.length > " + PROP_NAME_MAX);
        }
        return nativeGet(key, def);
    }

    private static String getString(String property) {
        return get(property, "unknown");
    }
}

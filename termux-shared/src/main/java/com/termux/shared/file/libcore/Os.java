package com.termux.shared.file.libcore;

public class Os {
    public static native String readlink(String path) throws ErrnoException;
    public static native void symlink(String oldPath, String newPath) throws ErrnoException;
    public static native StructStat stat(String path) throws ErrnoException;
    public static native StructStat lstat(String path) throws ErrnoException;
    public static native StructStat fstat(FileDescriptor fd) throws ErrnoException;

    static { System.loadLibrary("posix"); }
}

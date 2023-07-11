/**
*******************************************************************************
* Copyright (C) 1996-2005, International Business Machines Corporation and    *
* others. All Rights Reserved.                                                *
*******************************************************************************
*
*******************************************************************************
*/
package com.termux.shared.nio.charset;
import com.termux.shared.file.libcore.NativeConverter;
final class CharsetICU extends Charset {
    private final String icuCanonicalName;
    protected CharsetICU(String canonicalName, String icuCanonName, String[] aliases) {
         super(canonicalName, aliases);
         icuCanonicalName = icuCanonName;
    }
}

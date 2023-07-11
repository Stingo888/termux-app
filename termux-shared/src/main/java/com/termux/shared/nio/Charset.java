/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
package com.termux.shared.nio.charset;
import java.io.UnsupportedEncodingException;
import com.termux.shared.nio.charset.spi.CharsetProvider;
import java.util.HashMap;
import java.util.HashSet;
import java.util.ServiceLoader;
import java.util.Set;
import com.termux.shared.file.libcore.NativeConverter;
public abstract class Charset implements Comparable<Charset> {
    private static final HashMap<String, Charset> CACHED_CHARSETS = new HashMap<String, Charset>();
    private static final Charset DEFAULT_CHARSET = getDefaultCharset();
    private final String canonicalName;
    private final HashSet<String> aliasesSet;
    /**
     * Constructs a <code>Charset</code> object. Duplicated aliases are
     * ignored.
     *
     * @param canonicalName
     *            the canonical name of the charset.
     * @param aliases
     *            an array containing all aliases of the charset. May be null.
     * @throws IllegalCharsetNameException
     *             on an illegal value being supplied for either
     *             <code>canonicalName</code> or for any element of
     *             <code>aliases</code>.
     */
    protected Charset(String canonicalName, String[] aliases) {
        // Check whether the given canonical name is legal.
        checkCharsetName(canonicalName);
        this.canonicalName = canonicalName;
        // Collect and check each unique alias.
        this.aliasesSet = new HashSet<String>();
        if (aliases != null) {
            for (String alias : aliases) {
                checkCharsetName(alias);
                this.aliasesSet.add(alias);
            }
        }
    }
    private static void checkCharsetName(String name) {
        if (name.isEmpty()) {
            throw new IllegalCharsetNameException(name);
        }
        if (!isValidCharsetNameStart(name.charAt(0))) {
            throw new IllegalCharsetNameException(name);
        }
        for (int i = 1; i < name.length(); ++i) {
            if (!isValidCharsetNamePart(name.charAt(i))) {
                throw new IllegalCharsetNameException(name);
            }
        }
    }
    private static boolean isValidCharsetNameStart(char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
    }
    private static boolean isValidCharsetNamePart(char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                c == '-' || c == '.' || c == ':' || c == '_';
    }
private static Charset cacheCharset(String charsetName, Charset cs) {
        synchronized (CACHED_CHARSETS) {
            // Get the canonical name for this charset, and the canonical instance from the table.
            String canonicalName = cs.name();
            Charset canonicalCharset = CACHED_CHARSETS.get(canonicalName);
            if (canonicalCharset == null) {
                canonicalCharset = cs;
            }
            // Cache the charset by its canonical name...
            CACHED_CHARSETS.put(canonicalName, canonicalCharset);
            // And the name the user used... (Section 1.4 of http://unicode.org/reports/tr22/ means
            // that many non-alias, non-canonical names are valid. For example, "utf8" isn't an
            // alias of the canonical name "UTF-8", but we shouldn't penalize consistent users of
            // such names unduly.)
            CACHED_CHARSETS.put(charsetName, canonicalCharset);
            // And all its aliases...
            for (String alias : cs.aliasesSet) {
                CACHED_CHARSETS.put(alias, canonicalCharset);
            }
            return canonicalCharset;
        }
    }
    /**
     * Returns a {@code Charset} instance for the named charset.
     *
     * @param charsetName a charset name (either canonical or an alias)
     * @throws IllegalCharsetNameException
     *             if the specified charset name is illegal.
     * @throws UnsupportedCharsetException
     *             if the desired charset is not supported by this runtime.
     */
    public static Charset forName(String charsetName) {
        // Is this charset in our cache?
        Charset cs;
        synchronized (CACHED_CHARSETS) {
            cs = CACHED_CHARSETS.get(charsetName);
            if (cs != null) {
                return cs;
            }
        }
        if (charsetName == null) {
            throw new IllegalCharsetNameException(null);
        }
        // Is this a built-in charset supported by ICU?
        checkCharsetName(charsetName);
        cs = NativeConverter.charsetForName(charsetName);
        if (cs != null) {
            return cacheCharset(charsetName, cs);
        }
        // Does a configured CharsetProvider have this charset?
        for (CharsetProvider charsetProvider : ServiceLoader.load(CharsetProvider.class)) {
            cs = charsetProvider.charsetForName(charsetName);
            if (cs != null) {
                return cacheCharset(charsetName, cs);
            }
        }
        throw new UnsupportedCharsetException(charsetName);
    }
    /**
     * Determines whether the specified charset is supported by this runtime.
     *
     * @param charsetName
     *            the name of the charset.
     * @return true if the specified charset is supported, otherwise false.
     * @throws IllegalCharsetNameException
     *             if the specified charset name is illegal.
     */
    public static boolean isSupported(String charsetName) {
        try {
            forName(charsetName);
            return true;
        } catch (UnsupportedCharsetException ex) {
            return false;
        }
    }
    /**
     * Returns the canonical name of this charset.
     *
     * <p>If a charset is in the IANA registry, this will be the MIME-preferred name (a charset
     * may have multiple IANA-registered names). Otherwise the canonical name will begin with "x-"
     * or "X-".
     */
    public final String name() {
        return this.canonicalName;
    }
    /**
     * Returns the name of this charset for the default locale.
     *
     * <p>The default implementation returns the canonical name of this charset.
     * Subclasses may return a localized display name.
     */
    public String displayName() {
        return this.canonicalName;
    }
    /**
     * Returns true if this charset is known to be registered in the IANA
     * Charset Registry.
     */
    public final boolean isRegistered() {
        return !canonicalName.startsWith("x-") && !canonicalName.startsWith("X-");
    }
    /*
     * -------------------------------------------------------------------
     * Methods implementing parent interface Comparable
     * -------------------------------------------------------------------
     */
    /**
     * Compares this charset with the given charset. This comparison is
     * based on the case insensitive canonical names of the charsets.
     *
     * @param charset
     *            the given object to be compared with.
     * @return a negative integer if less than the given object, a positive
     *         integer if larger than it, or 0 if equal to it.
     */
    public final int compareTo(Charset charset) {
        return this.canonicalName.compareToIgnoreCase(charset.canonicalName);
    }
    /*
     * -------------------------------------------------------------------
     * Methods overriding parent class Object
     * -------------------------------------------------------------------
     */
    /**
     * Determines whether this charset equals to the given object. They are
     * considered to be equal if they have the same canonical name.
     *
     * @param obj
     *            the given object to be compared with.
     * @return true if they have the same canonical name, otherwise false.
     */
    @Override
    public final boolean equals(Object obj) {
        if (obj instanceof Charset) {
            Charset that = (Charset) obj;
            return this.canonicalName.equals(that.canonicalName);
        }
        return false;
    }
    /**
     * Gets the hash code of this charset.
     *
     * @return the hash code of this charset.
     */
    @Override
    public final int hashCode() {
        return this.canonicalName.hashCode();
    }
    /**
     * Gets a string representation of this charset. Usually this contains the
     * canonical name of the charset.
     *
     * @return a string representation of this charset.
     */
    @Override
    public final String toString() {
        return getClass().getName() + "[" + this.canonicalName + "]";
    }
    /**
     * Returns the system's default charset. This is determined during VM startup, and will not
     * change thereafter. On Android, the default charset is UTF-8.
     */
    public static Charset defaultCharset() {
        return DEFAULT_CHARSET;
    }
    private static Charset getDefaultCharset() {
        String encoding = System.getProperty("file.encoding", "UTF-8");
        try {
            return Charset.forName(encoding);
        } catch (UnsupportedCharsetException e) {
            return Charset.forName("UTF-8");
        }
    }
}

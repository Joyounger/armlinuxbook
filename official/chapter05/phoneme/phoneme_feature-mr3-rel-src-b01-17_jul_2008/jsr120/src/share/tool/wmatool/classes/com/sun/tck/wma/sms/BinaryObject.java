/*
 *   
 *
 * Copyright  1990-2008 Sun Microsystems, Inc. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt).
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 or visit www.sun.com if you need additional
 * information or have any questions.
 */

package com.sun.tck.wma.sms;

import com.sun.tck.wma.BinaryMessage;
import com.sun.tck.wma.MessageConnection;

/**
 * Implements an instance of a binary message.
 */
public class BinaryObject extends MessageObject implements BinaryMessage {

    /** Buffer that will hold the data payload. */
    private byte[] buffer;

    /**
     * Construct a binary specific message.
     * @param  addr the destination address of the message.
     */
    public BinaryObject(String addr) {
	super(MessageConnection.BINARY_MESSAGE, addr);
    }

    /**
     * Returns an array of bytes representing the payload data
     * of the message, or <code>null</code> if it isn't set.
     *
     * <p>A reference to the byte array of this binary message
     * is returned. It is the same for all calls to this method
     * made before <code>setPayloadData</code> is called the
     * next time.
     *
     * @return <code>null</code> if the data hasn't been set,
     *         or this message's payload data
     * @see #setPayloadData
     */
    public byte[] getPayloadData() {
        return buffer;
    }

    /**
     * Sets the payload data of this binary message. It may
     * be set to <code>null</code>.
     * <p>This method actually sets the reference to the byte array.
     * Changes made to this array subsequently affect this
     * <code>BinaryMessage</code> object's contents. Therefore, this array
     * should not be reused by the applications until the message is sent and
     * <code>MessageConnection.send</code> returned.
     * </p>
     * @param data payload data represented as a byte array
     * @see #getPayloadData
     */
    public void setPayloadData(byte[] data) {
        buffer = data;
    }

}



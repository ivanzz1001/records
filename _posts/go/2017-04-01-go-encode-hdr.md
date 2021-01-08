---
layout: post
title: 使用go来实现二进制序列化
tags:
- go-language
categories: go-language
description: 使用go来实现二进制序列化
---


本文简单给出一个示例，来展示在golang中如何实现二进制的序列化。

<!-- more -->


## 1. 二进制序列化
{% highlight string %}
package tlv

import(
	"encoding/binary"
	"bytes"
	"errors"
)

/*
 * Note: Here we must export all the fields， because we use the reflect to decode and encode header
 */
type PkgHdr struct{
	Magic uint32
	Version uint8                         //high 4 bits is major version, low 4 bits is minor version
	Size uint32 
	Validity uint32                       //current no use
	StyleSignature [16]byte
	Reserved uint32
}

type HdrVersion struct{
	Major uint8
	Minor uint8
}

const(
	_PKGHDR_MAGIC = 0xABCDDCBA

	_VERSION_MAJOR = 0
	_VERSION_MINOR = 1
	_PKGHDR_VERSION = uint8(_VERSION_MAJOR << 4) + uint8(_VERSION_MINOR & 0xF)
)
var(
	pkgHdrSize int
)

func init(){
	pkgHdrSize = binary.Size(PkgHdr{})
}

func GetHdrSize() int{
	return pkgHdrSize
}

func (*PkgHdr)GetHdrSize() int{
	return pkgHdrSize
}

func (hdr *PkgHdr)Encode()([]byte, error){
	hdr.Magic = _PKGHDR_MAGIC
	hdr.Version = _PKGHDR_VERSION
	hdr.Reserved = 0

	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.BigEndian, hdr)

	return buf.Bytes(), err
}

/*
 * Description: encode PkgHdr into buf. If cap(buf) is less than 'pkgHdrSize', binary.Write() may allocate
 * a new space to store the result, but sometimes it may not be proper for us.
 *
 * Note: Be careful to use this function, easily to cause some problems!!!
 */
func (hdr *PkgHdr)EncodeV2(buf []byte)(int, error){
	hdr.Magic = _PKGHDR_MAGIC
	hdr.Version = _PKGHDR_VERSION
	hdr.Reserved = 0

	if cap(buf) < pkgHdrSize{
		return 0, errors.New("buf too small")
	}

	b := bytes.NewBuffer(buf[:0])
	err := binary.Write(b, binary.BigEndian, hdr)
	if err != nil{
		return 0, err
	}

	return pkgHdrSize, nil
}

func (hdr *PkgHdr)GetMagic() uint32{
	return hdr.Magic
}

func (hdr *PkgHdr)GetVersion() HdrVersion{
	major := (hdr.Version >> 4) & 0xF
	minor := (hdr.Version & 0xF)

	return HdrVersion{
		Major: major,
		Minor: minor,
	}
}

func Decode(buf []byte)(*PkgHdr, error){
	pkgHdr := PkgHdr{}

	b := bytes.NewBuffer(buf)
	err := binary.Read(b, binary.BigEndian, &pkgHdr)

	if err != nil{
		return nil, err
	}

	if pkgHdr.Magic != _PKGHDR_MAGIC{
		return nil, errors.New("invalid package header")
	}else if pkgHdr.Version != _PKGHDR_VERSION{
		return nil, errors.New("unsupported package header version")
	}

	return &pkgHdr, nil
}
{% endhighlight %}







<br />
<br />
**[参看]：**





<br />
<br />
<br />


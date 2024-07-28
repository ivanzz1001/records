---
layout: post
title: Linux TCP MIB统计汇总
tags:
- tcpip
categories: tcpip
description: Linux TCP MIB统计汇总
---

本文讲述一下使用netstat命令来统计TCP的相关指标。

<!-- more -->

## 1. 通过netstat统计TCP相关指标

执行如下命令：
<pre>
# cat /proc/net/netstat | awk '(f==0) {name=$1; i=2; while ( i<=NF) {n[i] = $i; i++ }; f=1; next} (f==1){ i=2; while ( i<=NF){ printf "%s%s = %d\n", name, n[i], $i; i++}; f=0}'
TcpExt:SyncookiesSent = 0
TcpExt:SyncookiesRecv = 0
TcpExt:SyncookiesFailed = 0
TcpExt:EmbryonicRsts = 0
TcpExt:PruneCalled = 0
TcpExt:RcvPruned = 0
TcpExt:OfoPruned = 0
TcpExt:OutOfWindowIcmps = 0
TcpExt:LockDroppedIcmps = 0
TcpExt:ArpFilter = 0
TcpExt:TW = 188
TcpExt:TWRecycled = 0
TcpExt:TWKilled = 0
TcpExt:PAWSActive = 0
TcpExt:PAWSEstab = 1
TcpExt:DelayedACKs = 119968
TcpExt:DelayedACKLocked = 12
TcpExt:DelayedACKLost = 241
TcpExt:ListenOverflows = 0
TcpExt:ListenDrops = 0
TcpExt:TCPHPHits = 577385
TcpExt:TCPPureAcks = 105708
TcpExt:TCPHPAcks = 756724
TcpExt:TCPRenoRecovery = 0
TcpExt:TCPSackRecovery = 0
TcpExt:TCPSACKReneging = 0
TcpExt:TCPSACKReorder = 0
TcpExt:TCPRenoReorder = 0
TcpExt:TCPTSReorder = 0
TcpExt:TCPFullUndo = 0
TcpExt:TCPPartialUndo = 0
TcpExt:TCPDSACKUndo = 0
TcpExt:TCPLossUndo = 0
TcpExt:TCPLostRetransmit = 136
TcpExt:TCPRenoFailures = 0
TcpExt:TCPSackFailures = 0
TcpExt:TCPLossFailures = 0
TcpExt:TCPFastRetrans = 0
TcpExt:TCPSlowStartRetrans = 0
TcpExt:TCPTimeouts = 217
TcpExt:TCPLossProbes = 11
TcpExt:TCPLossProbeRecovery = 0
TcpExt:TCPRenoRecoveryFail = 0
TcpExt:TCPSackRecoveryFail = 0
TcpExt:TCPRcvCollapsed = 0
TcpExt:TCPBacklogCoalesce = 185
TcpExt:TCPDSACKOldSent = 14
TcpExt:TCPDSACKOfoSent = 0
TcpExt:TCPDSACKRecv = 10
TcpExt:TCPDSACKOfoRecv = 0
TcpExt:TCPAbortOnData = 0
TcpExt:TCPAbortOnClose = 5
TcpExt:TCPAbortOnMemory = 0
TcpExt:TCPAbortOnTimeout = 1
TcpExt:TCPAbortOnLinger = 0
TcpExt:TCPAbortFailed = 0
TcpExt:TCPMemoryPressures = 0
TcpExt:TCPMemoryPressuresChrono = 0
TcpExt:TCPSACKDiscard = 0
TcpExt:TCPDSACKIgnoredOld = 0
TcpExt:TCPDSACKIgnoredNoUndo = 6
TcpExt:TCPSpuriousRTOs = 0
TcpExt:TCPMD5NotFound = 0
TcpExt:TCPMD5Unexpected = 0
TcpExt:TCPMD5Failure = 0
TcpExt:TCPSackShifted = 0
TcpExt:TCPSackMerged = 0
TcpExt:TCPSackShiftFallback = 0
TcpExt:TCPBacklogDrop = 0
TcpExt:PFMemallocDrop = 0
TcpExt:TCPMinTTLDrop = 0
TcpExt:TCPDeferAcceptDrop = 0
TcpExt:IPReversePathFilter = 0
TcpExt:TCPTimeWaitOverflow = 0
TcpExt:TCPReqQFullDoCookies = 0
TcpExt:TCPReqQFullDrop = 0
TcpExt:TCPRetransFail = 0
TcpExt:TCPRcvCoalesce = 7797
TcpExt:TCPOFOQueue = 0
TcpExt:TCPOFODrop = 0
TcpExt:TCPOFOMerge = 0
TcpExt:TCPChallengeACK = 6
TcpExt:TCPSYNChallenge = 16
TcpExt:TCPFastOpenActive = 0
TcpExt:TCPFastOpenActiveFail = 0
TcpExt:TCPFastOpenPassive = 0
TcpExt:TCPFastOpenPassiveFail = 0
TcpExt:TCPFastOpenListenOverflow = 0
TcpExt:TCPFastOpenCookieReqd = 0
TcpExt:TCPFastOpenBlackhole = 0
TcpExt:TCPSpuriousRtxHostQueues = 30
TcpExt:BusyPollRxPackets = 0
TcpExt:TCPAutoCorking = 4121
TcpExt:TCPFromZeroWindowAdv = 1
TcpExt:TCPToZeroWindowAdv = 1
TcpExt:TCPWantZeroWindowAdv = 0
TcpExt:TCPSynRetrans = 171
TcpExt:TCPOrigDataSent = 901414
TcpExt:TCPHystartTrainDetect = 5
TcpExt:TCPHystartTrainCwnd = 463
TcpExt:TCPHystartDelayDetect = 0
TcpExt:TCPHystartDelayCwnd = 0
TcpExt:TCPACKSkippedSynRecv = 0
TcpExt:TCPACKSkippedPAWS = 0
TcpExt:TCPACKSkippedSeq = 0
TcpExt:TCPACKSkippedFinWait2 = 0
TcpExt:TCPACKSkippedTimeWait = 0
TcpExt:TCPACKSkippedChallenge = 10
TcpExt:TCPWinProbe = 0
TcpExt:TCPKeepAlive = 55
TcpExt:TCPMTUPFail = 0
TcpExt:TCPMTUPSuccess = 0
TcpExt:TCPDelivered = 901804
TcpExt:TCPDeliveredCE = 0
TcpExt:TCPAckCompressed = 0
TcpExt:TCPZeroWindowDrop = 0
TcpExt:TCPRcvQDrop = 0
TcpExt:TCPWqueueTooBig = 0
TcpExt:TCPFastOpenPassiveAltKey = 0
TcpExt:TcpTimeoutRehash = 216
TcpExt:TcpDuplicateDataRehash = 7
TcpExt:TCPDSACKRecvSegs = 10
TcpExt:TCPDSACKIgnoredDubious = 0
TcpExt:TCPMigrateReqSuccess = 0
TcpExt:TCPMigrateReqFailure = 0
TcpExt:TCPPLBRehash = 0
IpExt:InNoRoutes = 0
IpExt:InTruncatedPkts = 0
IpExt:InMcastPkts = 2105
IpExt:OutMcastPkts = 609
IpExt:InBcastPkts = 2027
IpExt:OutBcastPkts = 0
IpExt:InOctets = 1006205547
IpExt:OutOctets = 439956268
IpExt:InMcastOctets = 181331
IpExt:OutMcastOctets = 51771
IpExt:InBcastOctets = 404027
IpExt:OutBcastOctets = 0
IpExt:InCsumErrors = 0
IpExt:InNoECTPkts = 1607914
IpExt:InECT1Pkts = 0
IpExt:InECT0Pkts = 0
IpExt:InCEPkts = 0
IpExt:ReasmOverlaps = 0
MPTcpExt:MPCapableSYNRX = 0
MPTcpExt:MPCapableSYNTX = 0
MPTcpExt:MPCapableSYNACKRX = 0
MPTcpExt:MPCapableACKRX = 0
MPTcpExt:MPCapableFallbackACK = 0
MPTcpExt:MPCapableFallbackSYNACK = 0
MPTcpExt:MPFallbackTokenInit = 0
MPTcpExt:MPTCPRetrans = 0
MPTcpExt:MPJoinNoTokenFound = 0
MPTcpExt:MPJoinSynRx = 0
MPTcpExt:MPJoinSynAckRx = 0
MPTcpExt:MPJoinSynAckHMacFailure = 0
MPTcpExt:MPJoinAckRx = 0
MPTcpExt:MPJoinAckHMacFailure = 0
MPTcpExt:DSSNotMatching = 0
MPTcpExt:InfiniteMapTx = 0
MPTcpExt:InfiniteMapRx = 0
MPTcpExt:DSSNoMatchTCP = 0
MPTcpExt:DataCsumErr = 0
MPTcpExt:OFOQueueTail = 0
MPTcpExt:OFOQueue = 0
MPTcpExt:OFOMerge = 0
MPTcpExt:NoDSSInWindow = 0
MPTcpExt:DuplicateData = 0
MPTcpExt:AddAddr = 0
MPTcpExt:AddAddrTx = 0
MPTcpExt:AddAddrTxDrop = 0
MPTcpExt:EchoAdd = 0
MPTcpExt:EchoAddTx = 0
MPTcpExt:EchoAddTxDrop = 0
MPTcpExt:PortAdd = 0
MPTcpExt:AddAddrDrop = 0
MPTcpExt:MPJoinPortSynRx = 0
MPTcpExt:MPJoinPortSynAckRx = 0
MPTcpExt:MPJoinPortAckRx = 0
MPTcpExt:MismatchPortSynRx = 0
MPTcpExt:MismatchPortAckRx = 0
MPTcpExt:RmAddr = 0
MPTcpExt:RmAddrDrop = 0
MPTcpExt:RmAddrTx = 0
MPTcpExt:RmAddrTxDrop = 0
MPTcpExt:RmSubflow = 0
MPTcpExt:MPPrioTx = 0
MPTcpExt:MPPrioRx = 0
MPTcpExt:MPFailTx = 0
MPTcpExt:MPFailRx = 0
MPTcpExt:MPFastcloseTx = 0
MPTcpExt:MPFastcloseRx = 0
MPTcpExt:MPRstTx = 0
MPTcpExt:MPRstRx = 0
MPTcpExt:RcvPruned = 0
MPTcpExt:SubflowStale = 0
MPTcpExt:SubflowRecover = 0
MPTcpExt:SndWndShared = 0
MPTcpExt:RcvWndShared = 0
MPTcpExt:RcvWndConflictUpdate = 0
MPTcpExt:RcvWndConflict = 0

# netstat -s
</pre>








<br />
<br />

参看:


1. [Linux TCP MIB统计汇总](https://blog.csdn.net/mrpre/article/details/130423529)

2. [高级TCP指标](https://satori-monitoring.readthedocs.io/zh/latest/builtin-metrics/tcpext.html)



<br />
<br />
<br />


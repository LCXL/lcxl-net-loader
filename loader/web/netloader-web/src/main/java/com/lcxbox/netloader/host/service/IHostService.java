package com.lcxbox.netloader.host.service;

import java.io.IOException;
import java.net.UnknownHostException;

import com.lcxbox.common.model.CommonResponse;
import com.lcxbox.netloader.host.model.LcxlAddrInfo;
import com.lcxbox.netloader.host.model.ModuleListResponse;
import com.lcxbox.netloader.host.model.ServerListResponse;

public interface IHostService {
	public ModuleListResponse getModuleList() throws UnknownHostException, IOException;
	public ServerListResponse getServerList(long miniport_net_luid) throws UnknownHostException, IOException;
	public CommonResponse setVirtualAddr(long miniportNetLuid, LcxlAddrInfo virtualAddr) throws UnknownHostException, IOException;
}
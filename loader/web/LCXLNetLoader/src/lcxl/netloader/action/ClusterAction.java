package lcxl.netloader.action;

import java.util.ArrayList;
import java.util.List;

import lcxl.netloader.record.AppModuleInfo;

import com.opensymphony.xwork2.ActionSupport;

public class ClusterAction extends ActionSupport {
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	
	private List<AppModuleInfo> clusterList = new ArrayList<AppModuleInfo>();
	/**
	 * ��ü�Ⱥ�б�
	 * @return clusterList���ؼ�Ⱥ�б�
	 */
	public String clusterList() {
		
		return SUCCESS;
	}
	public List<AppModuleInfo> getClusterList() {
		return clusterList;
	}
}

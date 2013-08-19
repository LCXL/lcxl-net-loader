#ifndef _LCXL_FUNC_DELEGATE_H_
#define _LCXL_FUNC_DELEGATE_H_
//////////////////////////////////////////////////////////////////////////
//��ͷ�ļ�ʵ�������Ա������ί�й���
//Author: LCXL
//////////////////////////////////////////////////////////////////////////

///<summary>
///�¼�ģ�棬����ί��
///T:��ί���ߵ�class��������ί���ߵĶ���ð�䱻ί����
///E:ί�еĺ�������
///�÷���
///1.�����¼���
///     typedef void (A::*AEventCallBack)(XX XX, int x, int y);
///
///		class A {
///		public:
///			typedef _LCXLFunctionDelegate<A, AEventCallBack> AMessageEvent;
///     public:
///         void SetAMessageEvent(AMessageEvent &a_event) {
///               this->m_a_message_event = a_event;
///         }
///         //����A�¼�
///         void DelegateAEvent() {
///				(m_a_message_event.GetInvoker()->*m_a_message_event.Delegate())(0, 0);
///             //����Ϊ��TRIGGER_DELEGATE(m_a_message_event)(0, 0);
///         }  
///     private:
///         AMessageEvent m_a_message_event;
///     }
///
///2.ʹ��
///     class Host{
///		private:
///			A a;
///			void HostAEvent(int X, int Y);
///		public:
///			Host() {
///				A::AMessageEvent event(this, (AEventCallBack)&HostAEvent);
///             //�����¼�
///				a.SetAMessageEvent(event);
///				//�����¼�
///             a.DelegateAEvent();
///			}
///		}
///(XXX.GetInvoker()->*XXX.Delegate())(XXX, XXX);
///</summary>
template <typename T, typename E>
struct _LCXLFunctionDelegate {
	//������
	void * invoker;
	//���Ա�ص�����
	E delegate_func;
	_LCXLFunctionDelegate():invoker(NULL), delegate_func(NULL) {

	}
	_LCXLFunctionDelegate(void * invoker, E delegate_func):invoker(invoker),delegate_func(delegate_func)  {

	}
	T *GetInvoker() {
		return (T *)invoker;
	}
	//ί�к���
	E Delegate() {
		return delegate_func;
	}
	BOOL IsAvaliable() {
		return (invoker!=NULL&&delegate_func!=NULL);
	}
};
///<summary>
///����ί���ߺ����ĺ꣬�÷�ΪDELEGATE(_LCXLFunctionDelegate���͵ı���)(ί�к����Ĳ���)
///</summary>
#define TRIGGER_DELEGATE(__A) ((__A).GetInvoker()->*(__A).Delegate())

///<summary>
///����ί���ߺ����ĺ꣬�÷�ΪDELEGATE(_LCXLFunctionDelegate���͵ı���ָ��)(ί�к����Ĳ���)
///</summary>
#define TRIGGER_DELEGATE_P(__A) ((__A)->GetInvoker()->*(__A)->Delegate())

#endif
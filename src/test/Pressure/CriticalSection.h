// CriticalSection.h
//

//////////////////////////////////////////////////////////////////////////////////////////////////
//

class CriticalSeciton
{
public:
	CriticalSeciton() : mMux(portMUX_INITIALIZER_UNLOCKED) {
	}
	
	void EnterISR() { 
		portENTER_CRITICAL_ISR(&mMux); 
	}
	void LeaveISR() { 
		portEXIT_CRITICAL_ISR(&mMux); 
	}

	void Enter() { 
		portENTER_CRITICAL(&mMux); 
	}
	void Leave() { 
		portEXIT_CRITICAL(&mMux); 
	}
	
protected:
	portMUX_TYPE mMux;
};

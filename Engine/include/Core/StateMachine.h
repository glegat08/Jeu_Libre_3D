#pragma once 
#include <memory>
#include <vector>
#include "InputManager.h"
class StateMachine;

struct IState
{
	friend StateMachine;
	virtual ~IState();
	virtual void OnEnter() = 0;
	virtual void OnExit() = 0;
	virtual void Update(float dt) = 0;
	virtual void Input() = 0;
	virtual void Render() = 0;
protected:
	template<typename state, typename... Args> requires std::is_base_of_v<IState, state>
	void SetNextState(Args&&... args);
private:
	std::unique_ptr<IState> m_nextState = nullptr;
	std::unique_ptr<IState> GetNextState();
};

template <typename state, typename ... Args> requires std::is_base_of_v<IState, state>
void IState::SetNextState(Args&&... args)
{
	m_nextState = std::make_unique<state>(std::forward<Args>(args)...);
}

class StateMachine
{
public:
	StateMachine(std::unique_ptr<IState> state, size_t maxcount = 1);
	void Input();
	void Update(const float& dt);
	void Render();
	void ChangeState();
private:
	std::vector<std::unique_ptr<IState>> m_currentState;
	size_t m_maxcount;
};
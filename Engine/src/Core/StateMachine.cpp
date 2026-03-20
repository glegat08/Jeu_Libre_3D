#include "Core/StateMachine.h"
IState::~IState() = default;
std::unique_ptr<IState> IState::GetNextState()
{
	return std::move(m_nextState);
}

StateMachine::StateMachine(std::unique_ptr<IState> state, size_t maxcount): m_currentState(), m_maxcount(maxcount)
{
	m_currentState.push_back(std::move(state));
	m_currentState.back()->OnEnter();
}

void StateMachine::Input()
{
	m_currentState.back()->Input();
}

void StateMachine::Update(const float& dt)
{
	m_currentState.back()->Update(dt);
}

void StateMachine::Render()
{
	m_currentState.back()->Render();
}

void StateMachine::ChangeState()
{
	auto nextState = m_currentState.back()->GetNextState();
	if (!nextState)
		return;

	m_currentState.back()->OnExit();

	m_currentState.push_back(std::move(nextState));

	m_currentState.back()->OnEnter();

	if (m_currentState.size() > m_maxcount)
		m_currentState.erase(m_currentState.begin());
}

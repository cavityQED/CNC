#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include "common.h"

namespace CNC
{

class ControlPanel : public QWidget
{
	Q_OBJECT

public:

	ControlPanel(QWidget* parent = nullptr);
	~ControlPanel() {}

	void setupModeGroup();
	void setupArrowGroup();
	void setupJogGroup();
	void setupTextBox();

public:

	QTextEdit* textBox() {return m_textBox;}

signals:

	void modeChange(CNC::MODE mode);
	void axisButton(CNC::AXIS a, bool dir);
	void jogSpeed(int speed);


protected:

	QGroupBox*		m_mode_group;
	QGroupBox*		m_arrow_group;
	QGroupBox*		m_jog_group;
	QGroupBox*		m_control_group;

	QPushButton* 	m_jog_button;
	QPushButton* 	m_home_button;
	QPushButton* 	m_auto_button;
	QPushButton* 	m_MDI_button;
	QPushButton* 	m_edit_button;
	QPushButton* 	m_two_button;

	QPushButton*	m_up_button;
	QPushButton*	m_down_button;
	QPushButton*	m_left_button;
	QPushButton*	m_right_button;

	QPushButton*	m_jog100_button;
	QPushButton*	m_jog10_button;
	QPushButton*	m_jog1_button;
	QPushButton*	m_jogXpos_button;
	QPushButton*	m_jogXneg_button;
	QPushButton*	m_jogYpos_button;
	QPushButton*	m_jogYneg_button;
	QPushButton*	m_jogZpos_button;
	QPushButton*	m_jogZneg_button;

	QPushButton*	m_reset_button;
	QPushButton*	m_run_button;
	QPushButton*	m_hold_button;

	QTextEdit*		m_textBox;
};

}//CNC namespace

#endif
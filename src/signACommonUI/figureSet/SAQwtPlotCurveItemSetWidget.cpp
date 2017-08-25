#include "SAQwtPlotCurveItemSetWidget.h"
#include "qwt_plot_curve.h"
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include "SAPenSetWidget.h"
#include "SADoubleSpinBoxPropertyItem.h"
#include "SAQwtSymbolComboBox.h"
#include <QHBoxLayout>
#include <QMenu>
#include <QWidgetAction>
#include "SAQwtSymbolSetWidget.h"
#include <QScopedPointer>
#include <QCursor>
SAQwtPlotCurveItemSetWidget::SAQwtPlotCurveItemSetWidget(QwtPlotCurve *plotItem, QWidget *par)
    :SAQwtPlotItemSetWidget(plotItem,par)
    ,m_showAll(false)
    ,m_symbolSetMenu(nullptr)
    ,m_symbolSetWidget(nullptr)
{
    m_curveItem = plotItem;
    m_isSPlineCheckBox = nullptr;
    //set pen -> setPen
    m_PenStyle = new SAPenSetWidget();
    addWidget(m_PenStyle);
    //
    QHBoxLayout* hlayoutSymbolComboBox = new QHBoxLayout;
    m_labelSymbolSet = new QLabel;
    m_symbolComboBox = new SAQwtSymbolComboBox();
    m_symbolSetButton = new QPushButton;
    m_symbolSetButton->setMinimumWidth(15);
    m_symbolSetButton->setMaximumWidth(15);
    m_symbolSetButton->setText(">");
    hlayoutSymbolComboBox->addWidget(m_labelSymbolSet);
    hlayoutSymbolComboBox->addWidget(m_symbolComboBox);
    hlayoutSymbolComboBox->addWidget(m_symbolSetButton);
    addLayout(hlayoutSymbolComboBox);
    //update value
    upDateData();
    //connect
    connect(m_PenStyle,&SAPenSetWidget::penChanged
            ,this,&SAQwtPlotCurveItemSetWidget::onPenChenged);
    connect(m_symbolComboBox,&SAQwtSymbolComboBox::symbolSelectChanged
            ,this,&SAQwtPlotCurveItemSetWidget::onSymbolComboBoxChanged);
    connect(m_symbolSetButton,&QPushButton::clicked
            ,this,&SAQwtPlotCurveItemSetWidget::onSymbolSetButtonClicked);
    retranslateUi();
}

SAQwtPlotCurveItemSetWidget::~SAQwtPlotCurveItemSetWidget()
{

}

void SAQwtPlotCurveItemSetWidget::retranslateUi()
{
    SAQwtPlotItemSetWidget::retranslateUi();
    m_labelSymbolSet->setText(tr("Symbol Style"));
    if(m_showAll)
    {
        m_isSPlineCheckBox->setText(tr("fitter curve"));
        m_baseLineSpinBox->setText(tr("base line"));
    }
}

void SAQwtPlotCurveItemSetWidget::upDateData(bool downLoad)
{
    SAQwtPlotItemSetWidget::upDateData(downLoad);
    if(downLoad)
    {
        m_PenStyle->setPen(m_curveItem->pen());
        const QwtSymbol* s = m_curveItem->symbol();
        if(s)
        {
            m_symbolComboBox->setStyle(s->style());
        }
        else
        {
            m_symbolComboBox->setStyle(QwtSymbol::NoSymbol);
        }

        if(m_showAll)
        {
            m_isSPlineCheckBox->setChecked(m_curveItem->testCurveAttribute(QwtPlotCurve::Fitted));
            m_baseLineSpinBox->setValue(m_curveItem->baseline());
        }
    }
    else
    {
        onPenChenged(m_PenStyle->getPen());
        onSymbolComboBoxChanged(m_symbolComboBox->selectedSymbol());
        if(m_showAll)
        {
            onBaseLineSpinBoxValueChanged(m_baseLineSpinBox->getValue());
            onSPlineCheckBoxStateChanged(m_isSPlineCheckBox->checkState());
        }
    }
}

void SAQwtPlotCurveItemSetWidget::showAll(bool enable)
{
    m_showAll = enable;
    //fitter curve -> setCurveAttribute
    m_isSPlineCheckBox = new QCheckBox();
    m_isSPlineCheckBox->setToolTip(tr(""));
    addWidget(m_isSPlineCheckBox);
    //base Line
    m_baseLineSpinBox = new SADoubleSpinBoxPropertyItem();
    addWidget(m_baseLineSpinBox);
    //update value
    upDateData();
    //connect
    connect(m_isSPlineCheckBox,&QCheckBox::stateChanged
            ,this,&SAQwtPlotCurveItemSetWidget::onSPlineCheckBoxStateChanged);
    connect(m_baseLineSpinBox,&SADoubleSpinBoxPropertyItem::valueChanged
            ,this,&SAQwtPlotCurveItemSetWidget::onSymbolSetButtonClicked);

}

void SAQwtPlotCurveItemSetWidget::onPenChenged(const QPen &pen)
{
    m_curveItem->setPen(pen);
}

void SAQwtPlotCurveItemSetWidget::onSPlineCheckBoxStateChanged(int state)
{
    m_curveItem->setCurveAttribute(Qt::Checked == state
                                   ? QwtPlotCurve::Fitted
                                   : QwtPlotCurve::Inverted);

}

void SAQwtPlotCurveItemSetWidget::onBaseLineSpinBoxValueChanged(double v)
{
    m_curveItem->setBaseline(v);
}

void SAQwtPlotCurveItemSetWidget::onSymbolComboBoxChanged(QwtSymbol::Style style)
{
    m_symbolSetButton->setEnabled(QwtSymbol::NoSymbol != style);
    m_curveItem->setSymbol(new QwtSymbol(style));
}

void SAQwtPlotCurveItemSetWidget::onSymbolSetButtonClicked()
{
    if(nullptr == m_symbolSetMenu)
    {
        m_symbolSetMenu = new QMenu(this);
        QWidgetAction* act = new QWidgetAction(m_symbolSetMenu);
        QScopedPointer<QwtSymbol> symbol(new QwtSymbol(m_symbolComboBox->selectedSymbol()));
        m_symbolSetWidget = new SAQwtSymbolSetWidget(symbol.data());
        act->setDefaultWidget(m_symbolSetWidget);
        m_symbolSetMenu->addAction(act);
        connect(m_symbolSetMenu,&QMenu::aboutToHide
                ,this,&SAQwtPlotCurveItemSetWidget::onSymbolSetMenuHide);
    }
    m_symbolSetMenu->exec(QCursor::pos());
}

void SAQwtPlotCurveItemSetWidget::onSymbolSetMenuHide()
{
    if(m_symbolSetWidget)
    {
        const QwtSymbol& s = m_symbolSetWidget->getSymbol();
        QwtSymbol* symbol = new QwtSymbol(s.style(),s.brush(),s.pen(),s.size());
        m_curveItem->setSymbol(symbol);
        m_symbolComboBox->setStyle(symbol->style());
    }
}
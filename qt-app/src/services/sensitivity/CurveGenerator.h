#pragma once



#include <QObject>



#include "../../models/SensitivityPoint.h"

#include "../optimization/Optimizer.h"



class CurveGenerator : public QObject

{

    Q_OBJECT



public:

    explicit CurveGenerator(Optimizer* optimizer, QObject* parent = nullptr);



    SensitivityCurve generate(double minYield, double maxYield, double step) const;



signals:

    void progressChanged(int current, int total);



private:

    Optimizer* m_optimizer;

};


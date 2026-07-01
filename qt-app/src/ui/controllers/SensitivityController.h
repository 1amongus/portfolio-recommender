#pragma once



#include <QFutureWatcher>

#include <QObject>

#include <QString>

#include <QVariantList>

#include <QVariantMap>



#include "../../models/SensitivityPoint.h"

#include "../../services/data/DataStore.h"

#include "../../services/optimization/Optimizer.h"

#include "../../services/sensitivity/CurveGenerator.h"



class SensitivityController : public QObject

{

    Q_OBJECT

    Q_PROPERTY(double minYield READ minYield WRITE setMinYield NOTIFY minYieldChanged)

    Q_PROPERTY(double maxYield READ maxYield WRITE setMaxYield NOTIFY maxYieldChanged)

    Q_PROPERTY(double step READ step WRITE setStep NOTIFY stepChanged)

    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)

    Q_PROPERTY(QVariantList curveData READ curveData NOTIFY curveDataChanged)

    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)



public:

    explicit SensitivityController(QObject* parent = nullptr);

    ~SensitivityController() override = default;



    double minYield() const;

    void setMinYield(double minYield);



    double maxYield() const;

    void setMaxYield(double maxYield);



    double step() const;

    void setStep(double step);



    bool isLoading() const;

    int progress() const;

    QVariantList curveData() const;

    QString errorMessage() const;



    Q_INVOKABLE void generateCurve();

    Q_INVOKABLE QVariantMap pointDetails(int index) const;



signals:

    void minYieldChanged();

    void maxYieldChanged();

    void stepChanged();

    void isLoadingChanged();

    void progressChanged();

    void curveDataChanged();

    void errorMessageChanged();



private:

    void ensureSeedData();

    void setIsLoading(bool isLoading);

    void setProgress(int progress);

    void setErrorMessage(const QString& message);

    void updateCurveData(const SensitivityCurve& curve);



    double m_minYield;

    double m_maxYield;

    double m_step;

    bool m_isLoading;

    int m_progress;

    QVariantList m_curveData;

    QString m_errorMessage;

    SensitivityCurve m_lastCurve;

    DataStore m_dataStore;

    Optimizer m_optimizer;

    CurveGenerator m_curveGenerator;

    QFutureWatcher<SensitivityCurve> m_curveWatcher;

};


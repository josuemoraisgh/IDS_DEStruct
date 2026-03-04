#ifndef ADAPTIVE_STATE_H
#define ADAPTIVE_STATE_H

#include "adaptive_tuning_engine.h"
#include <QMutex>

///////////////////////////////////////////////////////////////////////////////
/// Estado adaptativo compartilhado entre threads.
/// Armazena parâmetros atualizados pelo motor de auto-tuning.
///////////////////////////////////////////////////////////////////////////////
struct AdaptiveState {
    // Parâmetros atualizados a cada 5 gerações
    AdaptiveParameters current_parameters;

    // Último snapshot calculado
    IndicatorSnapshot last_snapshot;

    // Controle de frequência de update
    qint64 last_update_generation = -10; // Garante que primeiro update ocorra

    // Fitness da melhor solução da geração anterior
    qreal previous_best_fitness = 0.0;

    // Lock para acesso seguro entre threads
    mutable QMutex mutex;

    /// Retorna true se é hora de atualizar (a cada 5 gerações)
    bool shouldUpdate(qint64 current_generation) const
    {
        return (current_generation - last_update_generation) >= 5;
    }

    /// Atualiza estado com novos parâmetros e snapshot
    void update(const AdaptiveParameters& params, const IndicatorSnapshot& snapshot, 
                qint64 generation)
    {
        QMutexLocker locker(&mutex);
        current_parameters = params;
        last_snapshot = snapshot;
        last_update_generation = generation;
    }

    /// Retorna cópia segura dos parâmetros atuais
    AdaptiveParameters getParameters() const
    {
        QMutexLocker locker(&mutex);
        return current_parameters;
    }

    /// Retorna cópia segura do último snapshot
    IndicatorSnapshot getLastSnapshot() const
    {
        QMutexLocker locker(&mutex);
        return last_snapshot;
    }
};

#endif // ADAPTIVE_STATE_H

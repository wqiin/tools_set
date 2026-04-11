
#ifndef QT_DELAYED_TIMER_HPP
#define QT_DELAYED_TIMER_HPP

#include <QTimer>

namespace Helper {

    class delayed_timer : public QTimer{
    public:
        using QTimer::QTimer;

        template<class slot_t>
        bool start(const std::uint32_t timeout_ms, slot_t slot)
        {
            if(timeout_ms <= 0 || m_interval > 0){
                return false;
            }

            m_interval = timeout_ms;
            setInterval(m_interval);
            setSingleShot(true);

            connect(this, &QTimer::timeout, slot, Qt::ConnectionType::UniqueConnection);

            return true;
        }

        bool restart()
        {
            bool stop_older_timer = false;

            if(QTimer::isActive()){
                QTimer::stop();
                stop_older_timer = true;
            }

            QTimer::start();

            return stop_older_timer;
        }

    protected:
        int m_interval = -1;
    };
}

#endif // QT_DELAYED_TIMER_HPP

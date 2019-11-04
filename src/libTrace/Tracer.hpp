#pragma once
#include <string>
#include <iostream>
#include <vector>

/// Tracer class which can be instantiated and which accepts sata to be streamed
/// into it mimicing the behavior of std::cout. However streamed data is only
/// forwareded to std::cout if the tracer has been enabled.
class Tracer
{
    public:
        /// Enables all currently existing tracers hich have a name starting with f_namePrefix.
        static void enableAllWithPrefix(const std::string & f_namePrefix)
        {
            for(Tracer * tracer : getTracerList())
            {
                if(tracer->getName().rfind(f_namePrefix, 0) == 0)
                {
                    tracer->enable();
                }
            }
        }

        /// Constructa a new tracer and registers it to the global list.
        Tracer(std::string f_name) :
            m_name(f_name)
        {
            getTracerList().push_back(this);
        }

        /// Enables a tracer. After it has been enabled streamed data is forwarded
        /// to std::cout
        void enable()
        {
            m_enabled = true;
        }

        std::string getName()
        {
            return m_name;
        }

        template<typename T>
        Tracer & operator<<(const T& f_t)
        {
            if(m_enabled)
            {
                std::cout << f_t;
            }
            return *this;
        }

        Tracer & operator<< (std::ostream& (*f)(std::ostream &))
        {
            if(m_enabled)
            {
                f(std::cout);
            }
            return *this;
        }

        Tracer & operator<< (std::ostream& (*f)(std::ios &))
        {
            if(m_enabled)
            {
                f(std::cout);
            }
            return *this;
        }

        Tracer& operator<< (std::ostream& (*f)(std::ios_base &))
        {
            if(m_enabled)
            {
                f(std::cout);
            }
            return *this;
        }

    private:
        std::string m_name;
        bool m_enabled = false;
        // 
        // FIXME: not really safe, only works if tracers are never deleted
        static std::vector<Tracer*> & getTracerList()
        {
            static std::vector<Tracer*> list;
            return list;
        }
};

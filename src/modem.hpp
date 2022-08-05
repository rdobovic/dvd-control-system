#ifndef _INCLUDE_MODEM_HPP_
#define _INCLUDE_MODEM_HPP_

class modem_manipulation {
    private:
        int start_counter;
    public:
        modem_manipulation();
        void init();
        void start();
        int ready();
};

#endif
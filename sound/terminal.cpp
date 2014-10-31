
#include "builders.hpp"

// ! make delayed_sum::entries be a vector instead of a set,
// because it can be assumed that the are always added in-order
// (of t).  have a check in c when push_back that t >= back().t
//
//write to nonblobking sink from generated units a and b.
//the running ug will always exist.
//
//toplevel ug, running, is a score of two entries starting
//at zero t.  one is the SONG that is playing.  the other is
//the HITS from the realtime user interface.  dont care
//to have the SONG for now.  build the
//score as normally, having a reference to the ug that we
//manipulate from realtime, appending generator entries as
//would normally have been done by its score builder.
//
//next step is to have a song going.  note that it will be
//attached to the toplevel score in parallell with the HITS
//score.  these are two only two inputs on the toplevel score.
//
// WELL, have a special PLUG generator on-top, because
// the delay-buffering and so is a waste on the SONG part,
// and the HITS part can just be a vector of generators appended
// realtime and removed after X appends, so it has maximum X
// entries at any time.  after a while most will return !more()
// and can then be removed just as if they was proxied by a lazy.
// the SONG part will be the root generator of the songs build()
// we take care of metro and touch (reflecting what is sent to
// sink at this point) adding to HITS score.  so we have this
// ug layout;
//
// LIMITER <---      SUM|s<--- DELAYED_SUM <== song.build()
//                      |k<--- DELAYED_SUM <== manipulated by us

bu_ptr kp(double f)
{
    double d = 9;
    bu_ptr s = P<strong>(P<fixed>(f), P<avg>());
    return P<attack>(0, 1, d, s);
}

int main()
{
    unsigned pos=0;
    unit a;
    unit b;


    te_global.running = P<sum>();

    int fd = te_global.sink;

    int ch=0;
    for(int i=0;i!=100000;++i){
    usleep(1000);
    ch = getch();
    if (ch!=ERR)
        if (ch==KEY_UP)break;
        if (ch==KEY_DOWN)
        {
            te_global.running.s.push_back(kp(400));
        }
        if (ch==KEY_RESIZE){
            int y,x;
            getmaxyx(stdscr,y,x);

            mvprintw(0,0,"%d,%d",y,x);
            refresh();
        }

        while (pos<SU) {

            int c=write(fd,a.y[pos],1);
            if (c==-1) {
                if (errno==EAGAIN)break;
                else exit(1);
            }
            pos+=c;
        }
        if (pos==SU) {
            a=b;
            te_global.running->generate(b);
        }
    }
    endwin();
    printf("%d\n",ch);
}


class DiningPhilosophers {
public:
    DiningPhilosophers() {  
    }

    void wantsToEat(int philosopher,
                    function<void()> pickLeftFork,
                    function<void()> pickRightFork,
                    function<void()> eat,
                    function<void()> putLeftFork,
                    function<void()> putRightFork) {
            int r = philosopher;
            int l = (philosopher+1)%5;
            if(philosopher % 2 == 0){
                forks[r].lock(); 
                forks[l].lock();
                pickLeftFork(); 
                pickRightFork();
            }
		    else{
                forks[l].lock();
                forks[r].lock();
                pickLeftFork(); 
                pickRightFork();
            }
            eat();
            putRightFork();
            putLeftFork();
            forks[l].unlock();
            forks[r].unlock();
        }
private:
    std::mutex forks[5];
};   

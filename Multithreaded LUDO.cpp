#include<ctime>
#include<sstream>
#include<iostream>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
using namespace std;

pthread_t tid[5];

// Semaphores to lock & unlock dice & Board
sem_t dice_lock;
sem_t board_lock;
// dice & board bool array help us to run random player
bool dice[4] = {false};
bool board[4] = {false};
//Save points location on board
int save_areas[8] = {0, 13, 26, 39, 47, 8, 34, 21};
string killed_token = "";
int no_of_py = 0;
string Board[52];

/* 
This is mini Stack which help us to
store store dice values and location of tokens,
which help us to move pawns/tokens
*/
class Ludo_Stack
{
    public:
        int *arr;
        int itr;
        Ludo_Stack()
        {
            arr = NULL;
            itr = 0;
        }

        void set_stack(int s)
        {
            arr = new int[s];
        }

        void push(int index)
        {
            arr[itr] = index;
            itr++;
        }

        bool empty(int s)
        {
            bool check = false;
            if(itr==0)
            {
                check = true;
            }else{
                for(int i=0; i<s; i++)
                {
                    arr[i] = 0;
                }
                itr = 0;
            }  
            return check;
        }

        void make_empty(int s)
        {
            for(int i=0; i<s; i++)
            {
                arr[i] = 0;
            }
            itr = 0;
        }

        void print()
        {
            cout<<"Stack : ";
            for(int i=0; i<itr; i++)
            {
                cout<<arr[i]<<" ";
            }
            cout<<endl;
        }
};

/*
Pawn struct that help us to store pawns colour 
& its location on Board
*/
struct Pawn
{
    string color = " ";
    int location = -1;
    bool on_home = false;    
};

/*
This is Players Class, Help us to manages activities 
of each player 
*/

class Ludo_player
{
    private:
        int kill_value;
        int start_index;
        int home_index;
        Ludo_Stack automatic_stack;

    public:
        int no_of_pawns;
        int player_id;
        string color;
        Pawn *pawns;
        string *save_home;   
        string *comp_pawns;
        Ludo_Stack dice_stack;
        int hit_rate;
        int kill_rate;
        int completed_tokens;

        Ludo_player()
        {
            completed_tokens = 0;
            hit_rate = 0;
            kill_rate = 0;
            pawns = NULL;
            player_id = 0;
            no_of_pawns = 0;
            color = "";
            kill_value = 0;  //Just for testing
            start_index = 0;
            home_index = 0;
            save_home = new string[5];
            for(int i=0; i<5; i++)
            {
                save_home[i] = " ";
            }
            comp_pawns = new string[4];
            for(int i=0; i<4; i++)
            {
                comp_pawns[i] = " ";
            }
        }

        /* This function use to set each player attributes */
        void set_up(int id, int p, string col, int start_id, int end_id)
        {
            no_of_pawns = p;
            player_id = id;
            color = col;
            start_index = start_id;
            home_index = end_id;
            dice_stack.set_stack(3);
            automatic_stack.set_stack(4);
            pawns = new Pawn[4];
            for(int i=0; i<no_of_pawns; i++)
            {
                pawns[i].color = color[0];
            }
            for(int i=no_of_pawns; i<4; i++)
            {
                pawns[i].color = " ";
                pawns[i].location = -20;
            }
        }

        /* This function use to select random location of each Player pawns to
        move in a Board */
        int auto_select()
        {
            int pawn_location = 0;
            srand(time(NULL)); 
            while(true)
            {
                int index = rand()%automatic_stack.itr;
                if(automatic_stack.arr[index]>-1)
                {
                    pawn_location = automatic_stack.arr[index];
                    automatic_stack.arr[index] = -1;
                    break;
                }
            }
            return pawn_location;
        }

        /* This function check is any pawn is on Board */
        bool is_pawns_open()
        {
            bool open_check = false;
            for(int i=0; i<no_of_pawns; i++)
            {
                if(pawns[i].location > -1)
                {
                    open_check = true;
                    cout<<color[0]<<" loc : "<<pawns[i].location<<endl;
                    automatic_stack.push(pawns[i].location);
                }
            }
            return open_check;
        }

        /* This function open a pawn, when dice value is 6 */
        bool open_player(bool six_checker)
        {
            for(int i=0; i<no_of_pawns; i++)
            {
                if(pawns[i].location==-1)
                {
                    six_checker = true;
                    pawns[i].location = start_index;
                    pawns[i].color = " ";
                    break;   
                }
            }
            return six_checker;
        }

        /* This function check is the given index of board is secured place or not */
        bool is_place_is_secured(int ind)
        {
            for(int i=0; i<8; i++)
            {
                if(save_areas[i]==ind)
                {
                    return true;
                }
            }
            return false;
        }

        /* This function move pawns on Board & also kill & move pawns in its home*/
        void move_pawns(int dice)
        {
            int location = auto_select();
            cout<<"Selected Location: "<<location<<endl;
            for(int i=0; i<no_of_pawns; i++)
            {
                if(pawns[i].location==location && !pawns[i].on_home)
                {
                    kill_rate++;
                    if(pawns[i].location+dice>home_index && pawns[i].location<=home_index && kill_value>=1)
                    {
                        int remaning = (pawns[i].location+dice)-home_index;
                        if(remaning>5)
                        {
                            for(int j=0; j<this->no_of_pawns; j++)
                            {
                                if(comp_pawns[j]==" ")
                                {
                                    comp_pawns[j] = color[0];
                                    completed_tokens += 1;
                                    pawns[i].location = -100;
                                    break;
                                }
                            }
                        }else{
                            pawns[i].on_home = true;
                            pawns[i].location = -20;
                            save_home[remaning-1] = color[0];
                            cout<<"Moved to Save Home"<<endl;
                        }
                        break;
                    }
                    string my_colour;
                    stringstream ss;
                    ss << color[0];
                    ss >> my_colour;
                    if(Board[pawns[i].location+dice] != " " && !is_place_is_secured(pawns[i].location+dice)
                    && Board[pawns[i].location+dice] != my_colour && pawns[i].location+dice<=51)
                    {   
                        kill_rate = 0;               
                        cout<<"Killed Token: "<<Board[pawns[i].location+dice]<<endl;
                        killed_token = Board[pawns[i].location+dice];
                        this->kill_value+=1;
                        pawns[i].location += dice;
                        break;
                    }
                    if(pawns[i].location+dice>51 && player_id!=1)
                    {
                        int remaining = (pawns[i].location+dice)-51;
                        pawns[i].location = remaining;
                        break;
                    }
                    if(pawns[i].location+dice>home_index && pawns[i].location<=home_index && kill_value==0)
                    {
                        break;
                    }
                    pawns[i].location += dice;
                    break;
                }
            }
        }

        /* This function check is any player in home array then move pawns to its 
        winning spot/triangle.
        */
        bool move_in_home(int dice)
        {
            bool home_checker = false;
            for(int j=0; j<5; j++)
            {
                if(save_home[j]!=" ")
                {
                    home_checker = true;
                    if(dice<6 && j<dice)
                    {
                        save_home[j] = " ";
                        save_home[dice-1] = color[0];
                    }else if(dice>0)
                    {
                        save_home[j] = " ";
                        for(int k=0; k<this->no_of_pawns; k++)
                        {
                            if(comp_pawns[k]==" ")
                            {
                                completed_tokens += 1;
                                comp_pawns[k] = color[0];
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            return home_checker;
        }

        /* This is the master function that use all above function to move pawns,
        kill and move them to its save location.
        */
        void auto_play()
        {
            for(int i=0; i<dice_stack.itr; i++)
            {
                int dice = dice_stack.arr[i];
                cout<<"Dice: "<<dice<<endl;
                bool six_checker = false;
                if(dice==6)
                {
                    six_checker = open_player(six_checker);
                }
                bool home_checker = false;
                if(dice<6 && dice>0  && kill_value>=1 || !six_checker)
                {
                    home_checker = move_in_home(dice);
                }
                if(dice<6 && dice>0 && !home_checker || !six_checker)
                {
                    cout<<endl;
                    bool pawns_checker = is_pawns_open();
                    if(pawns_checker==false)
                    {
                        hit_rate++;
                    }
                    cout<<endl;
                    if(pawns_checker)
                    {
                        hit_rate = 0;
                        move_pawns(dice);
                        cout<<"Total Kills: "<<kill_value<<endl;
                    }
                }
                dice_stack.arr[i] = 0;
                automatic_stack.make_empty(4);
            }
            dice_stack.make_empty(3);
            automatic_stack.make_empty(4);
        }

         /* This Function clear every things for a player reach its
        hit_rate or kill_rate limit
         */ 
        void clear()
        {
            for(int i=0; i<this->no_of_pawns; i++)
            {
                pawns[i].color = " ";
                pawns[i].location = -20;
            }
            string my_colour;
            stringstream ss;
            ss << color[0];
            ss >> my_colour;
            for(int i=0; i<52; i++)
            {
                if(Board[i]==my_colour)
                {
                    Board[i] = " ";
                }
            }
            for(int i=0; i<4; i++)
            {
                comp_pawns[i] = " ";
                save_home[i] = " ";
            }
            save_home[4] = " ";
        }
};

void Ludo_Board();
Ludo_player *players;
int total_player = 4;

/* This function move killed pawns to its Start Home */
void killed_token_callback(string tok)
{
    char temp = tok[0];
    int id = 0;
    for(int i=0; i<4; i++)
    {
        if(players[i].player_id != 0)
        {
            if(players[i].color[0]==temp)
            {
                id = i;
                break;
            }
        }
    }
    cout<<"Kill Value : "<<temp<<endl;
    cout<<"Id : "<<id<<endl;
    for(int i=0; i<no_of_py; i++)
    {
        if(players[id].pawns[i].color == " ")
        {
            players[id].pawns[i].color = tok;
            players[id].pawns[i].location = -1;
            break;
        }
    }
}

/* This function check is all player is diced to random play each thread */
int is_all_diced()
{
    int no = 0;
    for(int i=0; i<4; i++)
    {
        if(dice[i]==true)
        {
            no++;
        }
    }
    return no;
}

/* This function check is all player is moved to random play each thread */
int is_all_moved()
{
    int no = 0;
    for(int i=0; i<4; i++)
    {
        if(board[i]==true)
        {
            no++;
        }
    }
    return no;
}

void make_board();
/* In this Function we put Tokens on the Board */
void Fill_Board()
{
    make_board();
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<no_of_py; j++)
        {
            if(players[i].pawns[j].location>-1)
            {
                Board[players[i].pawns[j].location] = players[i].color[0];
            }
        }
    }
}

int pos = 0;
string winner[4];
int terminated[4] = {0};
int position_arr[4] = {0};


/* Player thread that play all players randomly */
void *player_thread(void *player_id)
{
    Ludo_player *player = (Ludo_player *) player_id;
    srand((unsigned)time(NULL));
    int counter = 0;
    bool win_check = false;
    while(true && terminated[player->player_id-1]==0 && total_player>1)
    {
        bool dice_checker = false;
        if(terminated[player->player_id-1]!=0)
        {
            break;
        }
        if(dice[player->player_id-1]==false && terminated[player->player_id-1]==0)
        {
            sem_wait(&dice_lock);
            while(true)
            {
                int dice_val = (rand()%6)+1;
                if(dice_val==6)
                {
                    player->dice_stack.push(dice_val);
                    counter++;
                }else if(dice_val<6)
                {
                    player->dice_stack.push(dice_val);
                    break;
                }
            }
            dice_checker = true;
            dice[player->player_id-1] = true;
            if(is_all_diced()>=total_player)
            {
                for(int i=0; i<4; i++)
                {
                    dice[i] = false;
                }
            }
            if(counter>=3)
            {
                counter = 0;
                player->dice_stack.make_empty(3);
                dice_checker = false;
            }
            sleep(3);
            sem_post(&dice_lock);
        }
        if(board[player->player_id-1]==false && dice_checker && terminated[player->player_id-1]==0)
        {
            sem_wait(&board_lock);
            counter = 0;

            cout<<"************"<<player->color<<endl;
            player->dice_stack.print();
            player->auto_play();
            if(killed_token!="")
            {
                killed_token_callback(killed_token);
                killed_token = "";
            }
            Fill_Board();
            board[player->player_id-1] = true;
            Ludo_Board();
            cout<<endl;
            if(is_all_moved()>=total_player)
            {
                for(int i=0; i<4; i++)
                {
                    board[i] = false;
                }
                cout<<"-------------------------------------------------"<<endl;
            }
            if(player->kill_rate>=20 || player->hit_rate>=20)
            {
                cout<<"Thread Canceled"<<endl;
                cout<<"Player "<<player->player_id<<" Color: "<<player->color<<endl;
                cout<<"Kill Rate: "<<player->kill_rate<<endl;
                cout<<"Hit Rate: "<<player->hit_rate<<endl;
                terminated[player->player_id-1] = 1; 
                total_player -= 1;
                cout<<"Player Remanining: "<<total_player<<endl;
                player->kill_rate = 0;
                player->hit_rate = 0;
                player->clear();
            }
            if(player->completed_tokens==no_of_py && terminated[player->player_id-1]==0)
            {
                cout<<"Winner"<<endl;
                cout<<"Player "<<player->player_id<<" Color: "<<player->color<<endl;
                total_player -= 1;
                cout<<"Player Remanining: "<<total_player<<endl;
                pos++;
                position_arr[player->player_id-1] = pos;
                winner[player->player_id-1] = player->color;
                terminated[player->player_id-1] = 1;
            }
            //sleep(1);
            sem_post(&board_lock);
        }
    }
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

/* Master thread that take input total pawns for each player
& and make player threads.
*/
void *master_thread(void *arg)
{
    cout<<"Enter no. of Tokens (max=4 or 1) : ";
    cin>>no_of_py;
    if(no_of_py==0)
    {
        cout<<"Please Enter Correct no. of Player's"<<endl;
        return 0;
    }else if(no_of_py>4)
    {
        cout<<"Max Range is 4, i changed your value from "<<no_of_py<<" to 4"<<endl;
        no_of_py = 4;
    }

    players =  new Ludo_player[4];
    string color[4] = {"Red", "Blue", "Green", "Yellow"};
    int enter_home[4] = {50, 37, 24, 11};
    int start_index[4] = {0, 39, 26, 13};

    for(int i=0; i<4; i++)
    {
        players[i].set_up(i+1, no_of_py, color[i], start_index[i], enter_home[i]);
    }
    
    sem_init(&dice_lock, 0, 1);
    sem_init(&board_lock, 0, 1);
    for(int i=0; i<4; i++)
    {
        pthread_create(&tid[i+1], NULL, player_thread, (void *)&players[i]);
    }   
    cout<<endl<<endl;
    for(int i=0; i<4; i++)
    {
        pthread_join(tid[i+1], NULL);
    }  
    cout<<endl<<endl;
    cout<<"Report"<<endl;
    for(int i=0; i<4; i++)
    {
        cout<<color[i]<<" Position "<<position_arr[i]<<endl;
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_create(&tid[0], NULL, master_thread, NULL);
    pthread_join(tid[0], NULL);

    pthread_exit(NULL);
    return 0;
}

void make_board()
{
    for(int i=0; i<52; i++)
    {
        Board[i] = " ";
    }
}

/* This Function print Board */
void Ludo_Board()
{
    cout << " _________________________________________________________________ " << endl;
    cout << "|           |           |__" << Board[36] << "__|__" << Board[37] << "__|__" << Board[38] << "__|           |           |" << endl;
    cout << "|     " << players[2].pawns[0].color << "     |     " << players[2].pawns[2].color << "     |__" << Board[35] << "__|__" << players[1].save_home[0] << "__|__" << Board[39] << "__|    " << players[1].pawns[0].color << "      |    " << players[1].pawns[2].color << "      |" << endl;
    cout << "|___________|___________|__" << Board[34] << "__|__" << players[1].save_home[1] << "__|__" << Board[40] << "__|___________|___________|" << endl;
    cout << "|           |           |__" << Board[33] << "__|__" << players[1].save_home[2] << "__|__" << Board[41] << "__|           |           |" << endl;
    cout << "|     " << players[2].pawns[3].color << "     |     " << players[2].pawns[1].color << "     |__" << Board[32] << "__|__" << players[1].save_home[3] << "__|__" << Board[42] << "__|    " << players[1].pawns[3].color << "      |     " << players[1].pawns[1].color << "     |" << endl;
    cout << "|___________|___________|__" << Board[31] << "__|__" << players[1].save_home[4] << "__|__" << Board[43] << "__|___________|___________|" << endl;
    cout << "|   |   |   |   |   |   |                 |   |   |   |   |   |   |" << endl;
    cout << "|_" << Board[25] << "_|_" << Board[26] << "_|_" << Board[27] << "_|_" << Board[28] << "_|_" << Board[29] << "_|_" << Board[30] << "_|    "<<players[1].comp_pawns[0]<<" "<<players[1].comp_pawns[1]<<" "<<players[1].comp_pawns[2]<<" "<<players[1].comp_pawns[3]<<"      |_" << Board[44] << "_|_" << Board[45] << "_|_" << Board[46] << "_|_" << Board[47] << "_|_" << Board[48] << "_|_" << Board[49] << "_|" << endl;
    cout << "|   |   |   |   |   |   |                 |   |   |   |   |   |   |" << endl;
    cout << "| " << Board[24] << " | " << players[2].save_home[0] << " | " << players[2].save_home[1] << " | " << players[2].save_home[2] << " | " << players[2].save_home[3] << " | " << players[2].save_home[4] << " |  "<<players[2].comp_pawns[0]<<players[2].comp_pawns[1]<<players[2].comp_pawns[2]<<players[2].comp_pawns[3]<<"    "<<players[0].comp_pawns[0]<<players[0].comp_pawns[1]<<players[0].comp_pawns[2]<<players[0].comp_pawns[3]<<"   | " << players[0].save_home[4] << " | " << players[0].save_home[3] << " | " << players[0].save_home[2] << " | " << players[0].save_home[1] << " | " << players[0].save_home[0] << " | " << Board[50] << " |" << endl;
    cout << "|___|___|___|___|___|___|    "<<players[3].comp_pawns[0]<<" "<<players[3].comp_pawns[1]<<" "<<players[3].comp_pawns[2]<<" "<<players[3].comp_pawns[3]<<"      |___|___|___|___|___|___|" << endl;
    cout << "|   |   |   |   |   |   |                 |   |   |   |   |   |   |" << endl;
    cout << "|_" << Board[23] << "_|_" << Board[22] << "_|_" << Board[21] << "_|_" << Board[20] << "_|_" << Board[19] << "_|_" << Board[18] << "_|_________________|_" << Board[4] << "_|_" << Board[3] << "_|_" << Board[2] << "_|_" << Board[1] << "_|_" << Board[0] << "_|_" << Board[51] << "_|" << endl;
    cout << "|           |           |__" << Board[17] << "__|__" << players[3].save_home[4] << "__|__" << Board[5] << "__|           |           |" << endl;
    cout << "|     " << players[3].pawns[0].color << "     |     " << players[3].pawns[2].color << "     |__" << Board[16] << "__|__" << players[3].save_home[3] << "__|__" << Board[6] << "__|    " << players[0].pawns[0].color << "      |    " << players[0].pawns[2].color << "      |" << endl;
    cout << "|___________|___________|__" << Board[15] << "__|__" << players[3].save_home[2] << "__|__" << Board[7] << "__|___________|___________|" << endl;
    cout << "|           |           |__" << Board[14] << "__|__" << players[3].save_home[1] << "__|__" << Board[8] << "__|           |           |" << endl;
    cout << "|     " << players[3].pawns[3].color << "     |     " << players[3].pawns[1].color << "     |__" << Board[13] << "__|__" << players[3].save_home[0] << "__|__" << Board[9] << "__|    " << players[0].pawns[3].color << "      |    " << players[0].pawns[1].color << "      |" << endl;
    cout << "|___________|___________|__" << Board[12] << "__|__" << Board[11] << "__|__" << Board[10] << "__|___________|___________|" << endl;
}
struct Transaction{
    int type;
    int amount;
    int to_from_type;
    Transaction(int=0, int=0, int=0);
};

Transaction :: Transaction(int type, int amount, int etc)
{
    this->type=type;
    this->amount=amount;
    this->to_from_type=etc;
}

int main(){
    Transaction t;
    return t.type;
}
Program ACS (Airline Checkin System aka thread runner) simulates an airline check-in system. The check-in sytem includes two queues (a priority queue and an economy queue) and 5 clerks.

How to use acs:
Step 1: Create customer text files 
Step 2: execute "make" in terminal to compile acs
Step 3: execute "./acs <customer file>" in terminal to run ACS

Note on customer text files:
Customer text files need to be in the format:
<number of customers>
<unique customer id>:<priority(1->priority, 0->economy)>,<arrival time(tenths of a second)>,<service time(tenths of a second)>
<unique customer id>:<priority(1->priority, 0->economy)>,<arrival time(tenths of a second)>,<service time(tenths of a second)>

The number of customers in line 1 needs to match the number of customers below. See sample.txt for example. 
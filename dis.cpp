#include <iostream>
#include <string>
#include <vector>
#include <fstream>

// OpCodes
enum OpCodes{R = 51, I = 19, IL = 3, IJALR = 103, S = 35, B = 99, J = 111, UI = 55, UPC = 23};

using namespace std;

// Instruction Printing Function
template <typename T>
void printVec(T v)
{
    int j = 0;
    for(int i = 0; i < v.size(); i++)
    {
        if(v[i][1] != '\033') 
        {
            cout << "\033[33m" << j*4 << "\033[0m" << ":\t" << v[i] << "" << endl;
            j = j + 1;
        }
        else cout << "\t" << v[i] << endl;
    }
    return;
}

// Generators
// Too Lazzyy ops
string genReg(int i)
{
    if((i < 0) || (i > 31)) return("BAD");

    string s = "";
    char c;

    if(i == 0)
    {
        return("x0");
    }

    while(i != 0)
    {
        c = '0' + (i%10);
        s = c + s;
        i = i/10;
    }

    s = 'x' + s;
    return(s);
}

string genImm(long int i)
{
    bool neg = false;
    if(i < 0)
    {
        i = -i;
        neg = true;
    }

    string s = "";
    char c;

    if(i == 0)
    {
        return("0");
    }

    while(i != 0)
    {
        c = '0' + (i%10);
        s = c + s;
        i = i/10;
    }

    if(neg) s = '-' + s;
    return(s);
}

string genImmHex(long int i)
{
    string s = "";
    char c;
    int dig;
    while(i != 0)
    {
        dig = i%16;
        if(dig < 10) c = '0' + dig;
        else c = 'a' + (dig - 10);

        s = c + s;
        i = i/16;
    }

    s = "0x" + s;
    return(s);
}

long int getBimm(int x)
{
    // Extract imm_12, imm_10_5 from imm1, (basically funct7)
    int comp = (127 << 25);
    int imm1 = (comp & x) >> 25;
    int imm_12 = (imm1 & 64) >> 6;
    int imm_10_5 = (imm1 & 63);

    // Extract imm_4_1, imm_11 from imm2 (rd)
    comp = (31 << 7);
    int imm2 = (comp & x) >> 7;
    int imm_4_1 = (imm2 & 30) >> 1;
    int imm_11 = (imm2 & 1);

    // imm = [imm_12][imm_11][imm_10_5][imm_4_1]0
    long int imm = (-imm_12 << 12) + (imm_11 << 11) + (imm_10_5 << 5) + (imm_4_1 << 1);

    return imm;
}

long int getJimm(int x)
{
    // Extract imm
    int comp = ((1024 * 1024) - 1) << 12;
    int immScrambled = (comp & x) >> 12;

    int imm_20 = (immScrambled & (1 << 19)) >> 19;
    int imm_10_1 = (immScrambled & (1023 << 9)) >> 9;
    int imm_11 = (immScrambled & (1 << 8)) >> 8;
    int imm_19_12 = (immScrambled & 511);

    long int imm = (-imm_20 << 20) + (imm_19_12 << 12) + (imm_11 << 11) + (imm_10_1 << 1);

    return imm;
}


// Hex Parsing
// Strip
string rstrip(string x)
{
        bool strip = true;
        string y = "";
        for(int i = x.size() - 1; i >= 0; i--)
        {
            if((x[i] != ' ') && (strip))
            {
                y = x[i] + y;
                strip = false;
            }
            else if(!strip)
            {
                y = x[i] + y;
            }
        }       
        return(y);
}

// isHexAlpha ?? (if c in (1, 2, ... 6))
int isHexAlpha(char c)
{
    int i1 = (c - 'a') + 1;
    int i2 = (c - 'A') + 1;

    if((i1 > 0) and (i1 < 7)) return i1;
    else if((i2 > 0) and (i2 < 7)) return i2;
    else return (0);
}

// isNum ?? (if c in (0, 1, ... 9)) else (-1)
int isNum(char c)
{
    int i = (c - '0');
    if((i >= 0) and (i < 10)) return i;
    else return(-1);
}

// Converts hex string to integer
int strhex(string s)
{
    // Answer and initialise loop
    int ans = 0;
    int i = s.size() - 1;
    int b = 1;
    int dig;
    
    for(; i >= 0; i--)
    {
        // Find possible digit
        dig = isNum(s[i]);
        
        // If digit is numeric (valid)
        if(dig != -1)
        {
            ans += b * dig;
        }

        // Else check if digit is alpha or invalid
        else
        {
            dig = isHexAlpha(s[i]);
            if(dig != 0)
            {
                ans += b * (dig + 9);
            }
            else
            {
                return(-1);
            }
        }

        // Increment base
        b = b << 4;
    }
    return(ans);
}


// Instruction Parsing
// Deal with R format
string printR(int x)
{
    // Extract funct7
    int comp = (127 << 25);
    int funct7 = (comp & x) >> 25;

    // Extract funct3
    comp = (7 << 12);
    int funct3 = (comp & x) >> 12;

    // Extract rs1
    comp = (31 << 15);
    int rs1 = (comp & x) >> 15;

    // Extract rs2
    comp = (31 << 20);
    int rs2 = (comp & x) >> 20;

    // Extract rd
    comp = (31 << 7);
    int rd = (comp & x) >> 7;

    // String to be printed
    string risc_v = "";
    switch(funct3)
    {
        case 0:
            if(funct7 == 0) risc_v += "add ";
            else if(funct7 == 32) risc_v += "sub ";
            else return("\033[31mERROR \033[0m: Invalid Funct7 for R-Format");
            break;

        case 1:
            if(funct7 == 0) risc_v += "sll ";
            else return("\033[31mERROR \033[0m: Invalid Funct7 for R-Format");
            break;

        case 2:
            if(funct7 == 0) risc_v += "slt ";
            else return("\033[31mERROR \033[0m: Invalid Funct7 for R-Format");
            break;

        case 3:
            if(funct7 == 0) risc_v += "sltu ";
            else return("\033[31mERROR \033[0m: Invalid Funct7 for R-Format");
            break;

        case 4:
            if(funct7 == 0) risc_v += "xor ";
            else return("\033[31mERROR \033[0m: Invalid Funct7 for R-Format");
            break;

        case 5:
            if(funct7 == 0) risc_v += "srl ";
            else if(funct7 == 32) risc_v += "sra ";
            else
            {
                return("\033[31mERROR \033[0m: Invalid Funct7 for R-Format");
            }
            break;

        case 6:
            if(funct7 == 0) risc_v += "or ";
            else return("\033[31mERROR \033[0m: Invalid Funct7 for R-Format");
            break;

        case 7:
            if(funct7 == 0) risc_v += "and ";
            else return("\033[31mERROR \033[0m: Invalid Funct7 for R-Format");
            break;

        default:
            return("\033[31mERROR \033[0m: Invalid Funct3 for R-Format");
    }

    if((genReg(rd) != "") && (genReg(rs1) != "") && (genReg(rs2) != ""))
    {
        risc_v = risc_v + genReg(rd) + ", " + genReg(rs1) + ", " + genReg(rs2);
        return(risc_v);
    }
    else
    {
        return("\033[31mERROR \033[0m: Invalid Registers");
    }
}

// Deal with I
string printI(int x)
{
    // Extract opcode
    int opcode = (127 & x);

    // Extract funct3
    int comp = (7 << 12);
    int funct3 = (comp & x) >> 12;

    // Extract rs1
    comp = (31 << 15);
    int rs1 = (comp & x) >> 15;

    // Extract rd
    comp = (31 << 7);
    int rd = (comp & x) >> 7;

    // Extract immediate
    comp = (4095 << 20);
    int imm = (comp & x) >> 20;

    // String to be printed
    int funct7;
    string risc_v = "";
    bool type = false;
    switch(opcode)
    {
        case 19:
            type = true;
            switch(funct3)
            {
            case 0:
                risc_v += "addi ";
                break;

            case 1:
                risc_v += "slli ";
                imm = imm & 31;
                break;

            case 2:
                risc_v += "slti ";
                break;

            case 3:
                risc_v += "sltiu ";
                break;

            case 4:
                risc_v += "xori ";
                break;

            case 5:
                funct7 = (imm & (127 << 5)) >> 5;
                if(funct7 == 0) risc_v += "srli ";
                else if(funct7 == 32) risc_v += "srai ";
                else
                {
                    return("\033[31mERROR \033[0m: Invalid Funct7 for I-Format");
                }
                imm = imm & 31;
                break;

            case 6:
                risc_v += "ori ";
                break;

            case 7:
                risc_v += "andi ";
                break;

            default:
                return("\033[31mERROR \033[0m: Invalid Funct3 for I-Format");                
            }
            break;

        case 3:
            switch(funct3)
            {
            case 0:
                risc_v += "lb ";
                break;

            case 1:
                risc_v += "lh ";
                break;

            case 2:
                risc_v += "lw ";
                break;

            case 3:
                risc_v += "ld ";
                break;

            case 4:
                risc_v += "lbu ";
                break;

            case 5:
                risc_v += "lhu ";
                break;

            case 6:
                risc_v += "lwu ";
                break;

            default:
                return("\033[31mERROR \033[0m: Invalid Funct3 for I-Format");
            }
            break;

        case 103:
            risc_v += "jalr ";
            break;

        default:
            return("\033[31mERROR \033[0m: Invalid Opcode");
    }

    if(type)
    {
        if((genReg(rd) != "") && (genReg(rs1) != ""))
        {
            risc_v = risc_v + genReg(rd) + ", " + genReg(rs1) + ", " + genImm(imm);
            return(risc_v);
        }
        else
        {
            return("\033[31mERROR \033[0m: Invalid Registers");
        }
    }
    else
    {
        if((genReg(rd) != "") && (genReg(rs1) != ""))
        {
            risc_v = risc_v + genReg(rd) + ", " + genImm(imm) + '(' + genReg(rs1) + ')';
            return(risc_v);
        }
        else
        {
            return("\033[31mERROR \033[0m: Invalid Registers");
        }
    }
}

// Deal with S format
string printS(int x)
{
    // Extract imm1
    int comp = (127 << 25);
    int imm1 = (comp & x) >> 25;

    // Extract funct3
    comp = (7 << 12);
    int funct3 = (comp & x) >> 12;

    // Extract rs1
    comp = (31 << 15);
    int rs1 = (comp & x) >> 15;

    // Extract rs2
    comp = (31 << 20);
    int rs2 = (comp & x) >> 20;

    // Extract imm2
    comp = (31 << 7);
    int imm2 = (comp & x) >> 7;

    // imm = [imm1][imm2]
    int imm = (imm1 << 5) + imm2;

    // String to be printed
    string risc_v = "";
    switch(funct3)
    {
        case 0:
            risc_v += "sb ";
            break;

        case 1:
            risc_v += "sh ";
            break;

        case 2:
            risc_v += "sw ";
            break;

        case 3:
            risc_v += "sd ";
            break;

        default:
            return("\033[31mERROR \033[0m: Invalid Funct3 for S-Format");
    }

    if((genReg(rs1) != "") && (genReg(rs2) != ""))
    {
        risc_v = risc_v + genReg(rs2) + ", " + genImm(imm) + '(' + genReg(rs1) + ')';
        return(risc_v);
    }
    else
    {
        return("\033[31mERROR \033[0m: Invalid Registers");
    }
}

// Deal with B format
string printB(int x)
{
    // Extract funct3
    int comp;
    comp = (7 << 12);
    int funct3 = (comp & x) >> 12;

    // Extract rs1
    comp = (31 << 15);
    int rs1 = (comp & x) >> 15;

    // Extract rs2
    comp = (31 << 20);
    int rs2 = (comp & x) >> 20;

    // imm = [imm_12][imm_11][imm_10_5][imm_4_1]0
    long int imm = getBimm(x);

    // String to be printed
    string risc_v = "";
    switch(funct3)
    {
        case 0:
            risc_v += "beq ";
            break;

        case 1:
            risc_v += "bne ";
            break;

        case 4:
            risc_v += "blt ";
            break;

        case 5:
            risc_v += "bge ";
            break;

        case 6:
            risc_v += "bltu ";
            break;

        case 7:
            risc_v += "bgeu ";
            break;

        default:
            return("\033[31mERROR \033[0m: Invalid Funct3 for B-Format");
    }

    if((genReg(rs1) != "") && (genReg(rs2) != ""))
    {
        risc_v = risc_v + genReg(rs1) + ", " + genReg(rs2) + ", ";
        return(risc_v);
    }
    else
    {
        return("\033[31mERROR \033[0m: Invalid Registers");
    }
}

// Deal with J format
string printJ(int x)
{
    // Extract rd
    long int comp = (31 << 7);
    int rd = (comp & x) >> 7;

    // Extract imm
    long int imm = getJimm(x);

    // Print
    if(genReg(rd) != "")
    {
        string risc_v = "jal " + genReg(rd) + ", ";
        return(risc_v);
    }
    else
    {
        return("\033[31mERROR \033[0m: Invalid Registers");
    }
}

// Deal with U format
string printU(int x)
{
    // Extract rd
    long int comp = (31 << 7);
    int rd = (comp & x) >> 7;

    // Extract imm
    comp = ((1024 * 1024) - 1) << 12;
    long int imm = (comp & x);

    // Extract opcode
    int opcode = (x & 127);

    // Identify lui/auipc
    string risc_v = "";
    if(opcode == 55) risc_v += "lui ";
    else if(opcode == 23) risc_v += "auipc ";
    else
    {
        // cout << "INVALID3" << endl;
        return("\033[31mERROR \033[0m: Invalid Registers");
    }  

    // Print
    if(genReg(rd) != "")
    {
        risc_v = risc_v + genReg(rd) + ", " + genImmHex(imm >> 12);
        return(risc_v);
    }
    else
    {
        return("\033[31mERROR \033[0m: Invalid Registers");
    }   
}

int main(int argc, char* argv[])
{
    // Get filename from terminal
    string fileName = argv[1];

    // Read file and store lines in vector
    vector<string> lines = {};
    fstream code(fileName);
    string line;
    if(code.is_open())
    {
        while(getline(code, line))
        {
            lines.push_back(rstrip(line));
        }

        // Transform to integers
        vector<int> instr = {};
        vector<int> labels = {};
        for(int j = 0; j < lines.size(); j++)
        {
            if(strhex(lines[j]) == -1)
            {
                cout << "\033[31mERROR \033[0m: Invalid Hexstring!\n\033[33mAt line #" << j+1 << "\033[0m" << endl;
                return(1);
            }
            instr.push_back(strhex(lines[j]));
            labels.push_back(0);
        }

        // Print instruction type
        vector<string> final = {};
        int opcode;
        string temp;
        int labelCount = 1;
        int jump;
        for(int k = 0; k < instr.size(); k++)
        {
            jump = 0;
            switch(opcode = instr[k] & 127)
            {
                case R:
                    temp = printR(instr[k]);
                    break;
                
                case I:
                    temp = printI(instr[k]);
                    break;
                
                case IL:
                    temp = printI(instr[k]);
                    break;

                case IJALR:
                    temp = printI(instr[k]);
                    break;

                case S:
                    temp = printS(instr[k]);
                    break;

                case B:
                    temp = printB(instr[k]);
                    jump = getBimm(instr[k])/4;
                    break;

                case J:
                    temp = printJ(instr[k]);
                    jump = getJimm(instr[k])/4;
                    break;

                case UI:
                    temp = printU(instr[k]);
                    break;

                case UPC:
                    temp = printU(instr[k]);
                    break;
                
                default:
                    temp = "DNF";
                    cout << "\033[31mERROR \033[0m: Invalid Opcode - " << opcode << endl;
                    break;
            }

            // Check jump value
            if(jump != 0)
            {
                // Check if within range
                if((k + jump >= 0) && (k + jump < instr.size()))
                {
                    if(labels[k + jump] == 0)
                    {
                        labels[k + jump] = labelCount;
                        labelCount++;
                    }
                }
                else
                {
                    cout << "\033[31mERROR \033[0m: Program out of bounds\n\033[33mAt line #" << k+1  << "\nOffset: ";
                    cout << jump * 4 << "\nProgram Size: " << instr.size() * 4  << "\nExpected Counter: " << (jump + k) * 4 << "\033[0m" << endl;
                    return 1;
                }
            }
            final.push_back(temp);
        }

        // Prefix/suffix labels
        for(int a = 0; a < final.size(); a++)
        {
            // Check if label has to be prefixed
            if(labels[a] != 0)
            {
                final[a] = "\033[32mL" + genImm(labels[a]) + ":\033[0m\t" + final[a];
            }
            else
            {
                final[a] = '\t' + final[a];
            }

            // Check if label has to be suffixed
            opcode = instr[a] & 127;
            if(opcode == 99)
            {
                final[a] += "\033[32mL" + genImm(labels[a + getBimm(instr[a])/4]) + "\033[0m";
            }
            else if (opcode == 111)
            {
                final[a] += "\033[32mL" + genImm(labels[a + getJimm(instr[a])/4]) + "\033[0m";
            }
        }
        cout << "\033[34m-----------------ASSEMBLY CODE-----------------\033[0m" << endl;
        printVec(final);
        cout << "\033[34m-----------------------------------------------\033[0m" << endl;
        code.close();
    }
    else
    {
        cout << "\033[31mERROR \033[0m\033[33m: The file " << fileName << " doesn't exist!\033[0m\n";
        code.close();
    }
    return 0;
}
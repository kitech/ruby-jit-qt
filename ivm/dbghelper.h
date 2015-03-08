#ifndef DBGHELPER_H
#define DBGHELPER_H

#if LLVM_DUMP_COLOR > 0
#define DUMP_COLOR(d) \
    do { qDebug()<<"begin ast dumping:"<<(d); (d)->dumpColor(); qDebug()<<"end ast dump:"<<(d); } while (0);

#define DUMP_IR(ir) \
    do { qDebug()<<"begin ir dumping:"<<(ir); (ir)->dump(); qDebug()<<"end ir dump:"<<(ir); } while (0);

#define PRINT_STATS(s) \
    do { qDebug()<<"begin stats dumping:"<<(s); (s)->PrintStats(); qDebug()<<"end stats dump:"<<(s); } while (0);
#else
#define DUMP_COLOR(d) do {} while (0);
#define DUMP_IR(ir) do {} while (0);
#define PRINT_STATS(s) do {} while (0);
#endif



#endif /* DBGHELPER_H */

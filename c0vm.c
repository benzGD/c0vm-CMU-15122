/**************************************************************************/
/*              COPYRIGHT Carnegie Mellon University 2023                 */
/* Do not post this file or any derivative on a public site or repository */
/**************************************************************************/
#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "lib/xalloc.h"
#include "lib/stack.h"
#include "lib/contracts.h"
#include "lib/c0v_stack.h"
#include "lib/c0vm.h"
#include "lib/c0vm_c0ffi.h"
#include "lib/c0vm_abort.h"

/* call stack frames */
typedef struct frame_info frame;
struct frame_info {
  c0v_stack_t S;   /* Operand stack of C0 values */
  ubyte *P;        /* Function body */
  size_t pc;       /* Program counter */
  c0_value *V;     /* The local variables */
};



//-------helper functions for push and pop
void push_int(c0v_stack_t S, int32_t i) {
  c0v_push(S, int2val(i));
}







int execute(struct bc0_file *bc0) {
  REQUIRES(bc0 != NULL);

  /* Variables */
  c0v_stack_t S = c0v_stack_new(); /* Operand stack of C0 values */
  ubyte *P = bc0->function_pool[0].code;      /* Array of bytes that make up the current function */
  size_t pc = 0;     /* Current location within the current byte array P */
  c0_value *V = xcalloc(bc0->function_pool[0].num_vars, sizeof(c0_value));   /* Local variables (you won't need this till Task 2) */
  (void) V;      // silences compilation errors about V being currently unused

  /* The call stack, a generic stack that should contain pointers to frames */
  /* You won't need this until you implement functions. */
  gstack_t callStack = stack_new();
  (void) callStack; // silences compilation errors about callStack being currently unused

  while (true) {

#ifdef DEBUG
    /* You can add extra debugging information here */
    fprintf(stderr, "Opcode %x -- Stack size: %zu -- PC: %zu\n",
            P[pc], c0v_stack_size(S), pc);
#endif

    switch (P[pc]) {

    /* Additional stack operation: */

    case POP: {
      pc++;
      c0v_pop(S);
      break;
    }

    case DUP: {
      pc++;
      c0_value v = c0v_pop(S);
      c0v_push(S,v);
      c0v_push(S,v);
      break;
    }

    case SWAP: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      c0_value v1 = c0v_pop(S);
      ASSERT(!c0v_stack_empty(S));
      c0_value v2 = c0v_pop(S);

      c0v_push(S, v1);
      c0v_push(S, v2);
      
      break;

    }


    /* Returning from a function.
     * This currently has a memory leak! You will need to make a slight
     * change for the initial tasks to avoid leaking memory.  You will
     * need to revise it further when you write INVOKESTATIC. */

    case RETURN: {
      int retval = val2int(c0v_pop(S));
      assert(c0v_stack_empty(S));
// Another way to print only in DEBUG mode
IF_DEBUG(fprintf(stderr, "Returning %d from execute()\n", retval));
      // Free everything before returning from the execute function!
      c0v_stack_free(S);
      free(V);

      //check if the callstack is not empty

      if (!stack_empty(callStack))
      {

        //restore the contents for f()

        frame* f = (frame*)pop(callStack);
        S = f->S;
        c0v_push(S, int2val(retval));
        P = f->P;
        pc = f->pc;
        V = f->V;


        free(f);
        break;




      }


      //if the callStack is empty means we can
      //safely return 

      stack_free(callStack, NULL);
      return retval;



    }


    /* Arithmetic and Logical operations */

    case IADD: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));

      push_int(S, v1 + v2);
      break;
    }
    
    case ISUB: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));
  
      push_int(S, v2 - v1);
      break;
    }

    case IMUL: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));
  
      push_int(S, v2*v1);
      break;
    }

    case IDIV: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      if (v1 == 0)
      {
        c0_arith_error("division by zero not allowed!");
      }
      
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));

      if (v2 == INT_MIN && v1 == -1)
      {
        c0_arith_error("illegal integer division!(INT_MIN DIVIDED BY -1)");
      }
      
      push_int(S, v2 / v1);
      break;
      
    }
    
    case IREM: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      if (v1 == 0)
      {
        c0_arith_error("division by zero not allowed!");
      }
      
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));
  
      if (v2 == INT_MIN && v1 == -1)
      {
        c0_arith_error("illegal integer division!(INT_MIN DIVIDED BY -1)");
      }
      
      push_int(S, v2 % v1);
      break;
      
      
    }
    
   
    case IAND: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));
  
      push_int(S, v2 & v1);
      break;
    }

    case IOR: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));
  
      push_int(S, v2 | v1);
      break;
    }
    
    case IXOR: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));
  
      push_int(S, v2 | v1);
      break;
      
    }
    
    case ISHR: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));
      if (v1 < 0 || v1 >= 32)
      {
        c0_arith_error("shifting error!");
      }
      
  
      push_int(S, v2 >> v1);
      break;
      
    }
    
    case ISHL: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));
      if (v1 < 0 || v1 >= 32)
      {
        c0_arith_error("shifting error!");
      }
      push_int(S, v2 << v1);
      break;
      
    }


    /* Pushing constants */

    case BIPUSH: {
      pc++;
      int32_t v =  (int32_t)(byte)P[pc];
      push_int(S, v);
      pc++;
      break;
    }

    case ILDC: {
      pc++;
      uint16_t c1 = (uint16_t)P[pc];
      pc++;
      uint16_t c2 = (uint16_t)P[pc];
      uint16_t index = (c1<<8) | c2;
      ASSERT(index < bc0->int_count);

      c0v_push(S, int2val(bc0->int_pool[index]));
      pc++;
      break;
      
    }
    
    case ALDC: {
      pc++;
      uint16_t c1 = (uint16_t)P[pc];
      pc++;
      uint16_t c2 = (uint16_t)P[pc];
      uint16_t index = (c1<<8) | c2;
      ASSERT(index < bc0->string_count);

      char* a = bc0->string_pool + index ;

  
      c0v_push(S, ptr2val((void*)a));
      pc++;
      break;
      
    }

    case ACONST_NULL: {
      pc++;
      c0v_push(S, ptr2val(NULL));
      break;
    }


    /* Operations on local variables */

    case VLOAD: {
      pc++;
      uint8_t index = P[pc];
      c0v_push(S, V[index]);
      pc++;
      break;

    }

    case VSTORE: {
      pc++;
      uint8_t index = P[pc];
      V[index] = c0v_pop(S);
      pc++;
      break;

    }


    /* Assertions and errors */

    case ATHROW: {
      pc++;
      char* e = (char*)val2ptr(c0v_pop(S));
      c0_user_error(e);
      break;
      
    }
    
    case ASSERT: {
      pc++;
      char* e = (char*)val2ptr(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      if (x == 0)
      {
        c0_assertion_failure(e);
      }
      break;

    }


    /* Control flow operations */

    case NOP: {
      pc++;
      break;
    }

    case IF_CMPEQ: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      c0_value v1 = c0v_pop(S);
      ASSERT(!c0v_stack_empty(S));
      c0_value v2 = c0v_pop(S);
      int16_t c1 = (int16_t)P[pc];
      pc++;
      int16_t c2 = (int16_t)P[pc];
      int16_t c3 = (c1<<8) | c2 ;
      if (val_equal(v1, v2))
      {
        pc = pc + c3 - 2;
      } else {
        pc++;
      }
      break;
      
    }
    
    case IF_CMPNE: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      c0_value v1 = c0v_pop(S);
      ASSERT(!c0v_stack_empty(S));
      c0_value v2 = c0v_pop(S);
      int16_t c1 = (int16_t)P[pc];
      pc++;
      int16_t c2 = (int16_t)P[pc];
      int16_t c3 = (c1<<8) | c2 ;
      if (!val_equal(v1, v2))
      {
        pc = pc + c3 - 2;
      } else {
        pc++;
      }
      break;
      
    }
    
    case IF_ICMPLT: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));
      int16_t c1 = (int16_t)P[pc];
      pc++;
      int16_t c2 = (int16_t)P[pc];
      int16_t c3 = (c1<<8) | c2 ;
      if (v2 < v1)
      {
        pc = pc + c3 - 2;
      } else {
        pc++;
      }
      break;
      
    }
    
    case IF_ICMPGE: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));
      int16_t c1 = (int16_t)P[pc];
      pc++;
      int16_t c2 = (int16_t)P[pc];
      int16_t c3 = (c1<<8) | c2 ;
      if (v2 >= v1)
      {
        pc = pc + c3 - 2;
      } else {
        pc++;
      }
      break;
      
    }
    
    case IF_ICMPGT: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));
      int16_t c1 = (int16_t)P[pc];
      pc++;
      int16_t c2 = (int16_t)P[pc];
      int16_t c3 = (c1<<8) | c2 ;
      if (v2 > v1)
      {
        pc = pc + c3 - 2;
      } else {
        pc++;
      }
      break;
      
    }
    
    case IF_ICMPLE: {
      pc++;
      ASSERT(!c0v_stack_empty(S));
      int32_t v1 = val2int(c0v_pop(S));
      ASSERT(!c0v_stack_empty(S));
      int32_t v2 = val2int(c0v_pop(S));
      int16_t c1 = (int16_t)P[pc];
      pc++;
      int16_t c2 = (int16_t)P[pc];
      int16_t c3 = (c1<<8) | c2 ;
      if (v2 <= v1)
      {
        pc = pc + c3 - 2;
      } else {
        pc++;
      }
      break;
      
    }
    
    case GOTO: {
      pc++;
      // ASSERT(!c0v_stack_empty(S));
      // int32_t v1 = val2int(c0v_pop(S));
      // ASSERT(!c0v_stack_empty(S));
      // int32_t v2 = val2int(c0v_pop(S));
      int16_t c1 = (int16_t)P[pc];
      pc++;
      int16_t c2 = (int16_t)P[pc];
      int16_t c3 = (c1<<8) | c2 ;
      pc = pc + c3 - 2;
     
      break;

    }


    /* Function call operations: */

    case INVOKESTATIC: {
      pc++;
      uint16_t c1 = (uint16_t)P[pc];
      pc++;
      uint16_t c2 = (uint16_t)P[pc];
      uint16_t index = (c1 << 8) | c2;

      //creating a local array for function g
    

      uint8_t Vg_num_args = bc0->function_pool[index].num_args;
      uint8_t Vg_num_vars = bc0->function_pool[index].num_vars;
      c0_value *Vg = xcalloc(Vg_num_vars, sizeof(c0_value));


      for (int8_t i = Vg_num_args -1; i >= 0 ; i--)
      {

        assert(i >= 0);
        Vg[i] = c0v_pop(S);
        //populating the local variable array for g()
      }



      pc++;  //increment pc to point to the next instruction
      //when g() returns

      //create a new frame struct pointer
      frame* Frm = xcalloc(1, sizeof(frame));
      Frm->S = S;
      Frm->P = P;
      Frm->pc = pc;
      Frm->V = V;
      push(callStack, (void*)Frm);

      //save all the info of f() here and push
      //it into the global callStack
      //---------------------------------


      //----- adjust P and pc to the new function g
      S = c0v_stack_new();  // new(empty stack) for g
      P = bc0->function_pool[index].code; //P now counts to the code of g()
      pc = 0;  //reset the pc (relative pc)
      V = Vg ; //old V becomes new Vg


      break;

    }

    case INVOKENATIVE: {
      pc++;
      uint16_t c1 = (uint16_t)P[pc];
      pc++;
      uint16_t c2 = (uint16_t)P[pc];
      uint16_t index = (c1 << 8) | c2;



      uint16_t Vg_num_args = bc0->native_pool[index].num_args;
      uint16_t indx = bc0->native_pool[index].function_table_index;

      c0_value *Vg = xcalloc(Vg_num_args, sizeof(c0_value));
      for (uint16_t i = 0; i < Vg_num_args ; i++)
      {
        Vg[Vg_num_args -1 - i] = c0v_pop(S);
        //populating the local variable array for g()
      }

      c0v_push(S, (*native_function_table[indx])(Vg));

      free(Vg);

      pc++;

      break;

    }



    /* Memory allocation and access operations: */

    case NEW:

    case IMLOAD:

    case IMSTORE:

    case AMLOAD:

    case AMSTORE:

    case CMLOAD:

    case CMSTORE:

    case AADDF:


    /* Array operations: */

    case NEWARRAY:

    case ARRAYLENGTH:

    case AADDS:


    /* BONUS -- C1 operations */

    case CHECKTAG:

    case HASTAG:

    case ADDTAG:

    case ADDROF_STATIC:

    case ADDROF_NATIVE:

    case INVOKEDYNAMIC:

    default:
      fprintf(stderr, "invalid opcode: 0x%02x\n", P[pc]);
      abort();
    }
  }

  /* cannot get here from infinite loop */
  assert(false);
}

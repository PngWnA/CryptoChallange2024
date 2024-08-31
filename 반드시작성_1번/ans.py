import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import matplotlib.pyplot as plt
import time
from tqdm import tqdm

class DLFSR(nn.Module):

  def __init__(self,input_size):
    super(DLFSR,self).__init__()
    self.l1 = nn.Linear(input_size,2*input_size)
    self.l2 = nn.Linear(2*input_size,2*input_size)
    self.l3 = nn.Linear(2*input_size,2*input_size)
    self.bn1 = nn.BatchNorm1d(2*input_size)
    self.bn2 = nn.BatchNorm1d(2*input_size)
    self.bn3 = nn.BatchNorm1d(2*input_size)
    self.l4 = nn.Linear(2*input_size,2)

  def forward(self,x):
    x = F.relu(self.l1(x))
    x = self.bn1(x)
    x = F.relu(self.l2(x))
    x = self.bn2(x)
    x = F.relu(self.l3(x))
    x = self.bn3(x)
    x = F.relu(self.l4(x))
    x = F.softmax(x,dim=-1)
    return x
  
class DLF():

    def __init__(self,bit_size):
        self.bit_size = bit_size
        self.lfsr = DLFSR(bit_size).cuda()
        self.optimizer = optim.Adam(self.lfsr.parameters(),lr=1e-5, weight_decay=2e-1)

    def train(self,data,test_data,epoch=20,batch_size=256):
        self.data_x,self.data_y = data
        self.data_val_x,self.data_val_y = test_data
        self.data_x = self.data_x.cuda()
        self.data_y = self.data_y.cuda()
        self.data_val_x = self.data_val_x.cuda()
        self.data_val_y = self.data_val_y.cuda()
        self.lfsr.train()
        for k in range(epoch):
            loss = self.train_epoch(batch_size)
        self.lfsr.eval()
        train_acc = self.cal_acc(self.data_x,self.data_y)
        validation_acc = self.cal_acc(self.data_val_x,self.data_val_y)
        return loss, train_acc, validation_acc
        
    def train_epoch(self,batch_size):
        size = self.data_x.size(0)
        perm = torch.randperm(size)
        x_=self.data_x[perm]
        y_=self.data_y[perm]
        self.lfsr.eval()
        self.lfsr.train()
        for i in (range(0,size,batch_size)):
            loss = self.train_step(x_[i:i+batch_size],y_[i:i+batch_size])
        return loss

    def train_step(self,x,y):
        self.optimizer.zero_grad()
        pred = self.lfsr(x)
        loss = F.binary_cross_entropy(pred,y)
        loss.backward()
        self.optimizer.step()
        return loss
    
    def preprocess_stream_data(self,stream,val=0.8):
        print('DATA preprocess start')
        data_ = torch.Tensor(stream)
        data_ = data_.to(torch.int64)
        data_hot = torch.nn.functional.one_hot(data_,num_classes=2)
        data_x,data_y=[],[]
        for i in tqdm(range(len(stream)-self.bit_size)):
            data_x.append(data_[i:i+self.bit_size].to(torch.float32))
        data_y.append(data_hot[i+self.bit_size].to(torch.float32))
        data_x_total = torch.stack(data_x)
        data_y_total = torch.stack(data_y)
        data_x = data_x_total[:int(len(data_x_total)*val)]
        data_y = data_y_total[:int(len(data_x_total)*val)]
        data_test_x = data_x_total[int(len(data_x_total)*val):]
        data_test_y = data_y_total[int(len(data_x_total)*val):]
        data_ = (data_x,data_y)
        data_test_ = (data_test_x,data_test_y)
        return data_,data_test_
    
    def cal_acc(self,data_x,data_y):
        train_pred = self.lfsr(data_x)
        _, train_pred = torch.max(train_pred,dim=-1)
        acc = float((train_pred==torch.argmax(data_y,dim=-1)).sum()/len(self.data_y)*100)
        return acc
    
def predict_period(stream:list,bit_size,val):
    model = DLF(bit_size)
    data_train, data_test = model.preprocess_stream_data(stream,val)
    init_stream = stream[:bit_size]
    state = stream[:bit_size]
    test_acc = 0
    i=0
    it = 0
    start_time = time.time()
    while True:
        loss, train_acc, test_acc = model.train(data_train,data_test)
        print('{}th iter: train acc = {:.3f}, test acc = {}'.format(it+1,train_acc,test_acc),end='\r')
        if test_acc > 100-(1e-12):
            i+=1
        else:
            i = 0 
        it += 1
        if i == 50 or (time.time()-start_time)>300:
            break
    print('\ntrain done')
    model.lfsr.eval()

    end = False
    j = 0

    while not end:
        state_tensor = torch.Tensor(state).unsqueeze(0).to(torch.float32).cuda()
        pred_bit = model.lfsr(state_tensor)[0].argmax(0).item()
        state = state[1:]
        state.append(pred_bit)
        if init_stream == state:
            end = True
        j += 1

        if j>2**bit_size:
            print('failed')
            break

    return j
# obs-studio-simply-c-sdk
本项目是基于obs-studio开源项目  
并对其中的libobs代码进行二次开发并封装的c接口的sdk  
这是上传的还是去年七月份写的测试代码，

将于近期更新为去年11月份的代码，  
后来由于忙于另外一个python项目,  
所以到现在也没再去更新,  
不过接下来将会花时间打算好好完善它。  

写这个的起因，是因为打算基于obs做款游戏直播软件，  
但是obs studio的libobs不太好用，  
过于繁杂，故打算在其基础上重新写个c的dll, 来封装它。  
再自己另用qt写个界面，调用自己的c sdk.  
   
这个项目是基于vs2013, 在网上下载的obs studio的项目中里，添加进此项目。  
原来的obs studio是由cmake生成vs2013下的工程。  
生成的工程属性中，路径都是硬编码，所以我这个项目中暂时也是的硬编码。  
所以现在这个项目是无法正常编译的，你需要自己去修改这个项目的工程属性。  
后续我会慢慢改进。
   
最近由于工作忙，抱歉。  
但是或许忙有的时候也只是借口:(
   
上传此项目的目的，也希望那些和我一样，需要基于obs studio做二次开发的人，能节省的时间，更快的了解如何学习使用obs studio进行二次开发。  
少走点我走过的弯路，也就值得了。

我本人比较菜，也不擅长写sdk, 献丑了。

2017/2/22
   
> 如果一件事情，能使别人受益，那就值得去做。不是么。共勉...
>    
> 一粒小芝麻
>
> purpledaisy@163.com
>
> [purpledaisy.cn](http://purpledaisy.cn/)

 ----------------------------------------------------------------
 update 2020.9.24
这个项目的主要代码都是2016年写的。 
当时obs studio的资料还比较少。
还记得当时obs的官网上，官方说的obs很简单的，看代码就好了。当时看到这个回复，有点哭笑不得。
四年过去了，现在obs的网上资料也多了很多。
不过这半年里，也偶有些朋友来问我这个项目的事情，
没想到还能帮助到一些人。

这个项目暂时没有更新的打算了。
基本功能也都具备，可以用于作为obs开发的入门学习。
今天上传了一个当时写的demo.py.

-----------------------------
2023.8.25<p>

```diff
!keep fukushima radioactive water on the ground


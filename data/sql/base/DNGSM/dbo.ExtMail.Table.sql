USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtMail]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtMail](
	[id] [int] NULL,
	[_MailType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MailSender] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MailTitle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MailText] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MailPresentItem1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Count1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MailPresentItem2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Count2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MailPresentItem3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Count3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MailPresentItem4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Count4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MailPresentItem5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Count5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MailPresentMoney] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_IsCash] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_KeepBoxReceive] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_KeepBoxType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_KeepBoxLevelMin] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_KeepBoxLevelMax] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_KeepBoxClass] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MailTab] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtMail_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtMail_id] ON [dbo].[ExtMail]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

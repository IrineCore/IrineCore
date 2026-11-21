USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtQuest]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtQuest](
	[id] [int] NULL,
	[_Name] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_QuestType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_QuestLv] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_QuestTalk] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_QuestScript] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_QuestImageTitle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_QSortIndex] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_QuestRepeatable] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_IsEvent] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_QuestMarkHide] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Map] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_LevelCapReward] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_guide] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtQuest_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtQuest_id] ON [dbo].[ExtQuest]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

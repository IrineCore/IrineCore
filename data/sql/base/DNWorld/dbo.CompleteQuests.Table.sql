USE [dnworld]
GO
/****** Object:  Table [dbo].[CompleteQuests]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CompleteQuests](
	[CharacterID] [bigint] NOT NULL,
	[QuestID] [int] NOT NULL,
	[CompleteDate] [datetime2](0) NOT NULL,
	[CompleteCount] [int] NOT NULL,
	[QuestCode] [tinyint] NOT NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_CompleteQuests_CompleteDate]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE CLUSTERED INDEX [IX_NC_CompleteQuests_CompleteDate] ON [dbo].[CompleteQuests]
(
	[CompleteDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_CompleteQuests]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[CompleteQuests] ADD  CONSTRAINT [PK_CompleteQuests] PRIMARY KEY NONCLUSTERED 
(
	[CharacterID] ASC,
	[QuestID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[CompleteQuests] ADD  CONSTRAINT [DF_CompleteQuests_CompleteCount]  DEFAULT ((1)) FOR [CompleteCount]
GO
ALTER TABLE [dbo].[CompleteQuests] ADD  CONSTRAINT [DF_CompleteQuests_QuestCode]  DEFAULT ((1)) FOR [QuestCode]
GO

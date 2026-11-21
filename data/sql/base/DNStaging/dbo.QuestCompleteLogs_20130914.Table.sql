USE [dnstaging]
GO
/****** Object:  Table [dbo].[QuestCompleteLogs_20130914]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[QuestCompleteLogs_20130914](
	[CompleteDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[QuestID] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_QuestCompleteLogs_20130914_CompleteDate]    Script Date: 11/14/2026 9:00:07 AM ******/
CREATE CLUSTERED INDEX [IX_NC_QuestCompleteLogs_20130914_CompleteDate] ON [dbo].[QuestCompleteLogs_20130914]
(
	[CompleteDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_QuestCompleteLogs_20130914_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_QuestCompleteLogs_20130914_CharacterID] ON [dbo].[QuestCompleteLogs_20130914]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[QuestCompleteLogs_20130914]  WITH CHECK ADD  CONSTRAINT [CK_QuestCompleteLogs_20130914_CompleteDate] CHECK  (([CompleteDate]>='2013-09-14' AND [CompleteDate]<'2013-09-15'))
GO
ALTER TABLE [dbo].[QuestCompleteLogs_20130914] CHECK CONSTRAINT [CK_QuestCompleteLogs_20130914_CompleteDate]
GO

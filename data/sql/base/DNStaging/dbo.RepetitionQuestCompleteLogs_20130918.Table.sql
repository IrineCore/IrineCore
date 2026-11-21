USE [dnstaging]
GO
/****** Object:  Table [dbo].[RepetitionQuestCompleteLogs_20130918]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[RepetitionQuestCompleteLogs_20130918](
	[CompleteDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[QuestID] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_RepetitionQuestCompleteLogs_20130918_CompleteDate]    Script Date: 11/14/2026 9:00:07 AM ******/
CREATE CLUSTERED INDEX [IX_NC_RepetitionQuestCompleteLogs_20130918_CompleteDate] ON [dbo].[RepetitionQuestCompleteLogs_20130918]
(
	[CompleteDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_RepetitionQuestCompleteLogs_20130918_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_RepetitionQuestCompleteLogs_20130918_CharacterID] ON [dbo].[RepetitionQuestCompleteLogs_20130918]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[RepetitionQuestCompleteLogs_20130918]  WITH CHECK ADD  CONSTRAINT [CK_RepetitionQuestCompleteLogs_20130918_CompleteDate] CHECK  (([CompleteDate]>='2013-09-18' AND [CompleteDate]<'2013-09-19'))
GO
ALTER TABLE [dbo].[RepetitionQuestCompleteLogs_20130918] CHECK CONSTRAINT [CK_RepetitionQuestCompleteLogs_20130918_CompleteDate]
GO

USE [dnstaging]
GO
/****** Object:  Table [dbo].[MissionAchieveLogs_20131007]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MissionAchieveLogs_20131007](
	[AchieveDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[MissionID] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_MissionAchieveLogs_20131007_AchieveDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_MissionAchieveLogs_20131007_AchieveDate] ON [dbo].[MissionAchieveLogs_20131007]
(
	[AchieveDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_MissionAchieveLogs_20131007_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_MissionAchieveLogs_20131007_CharacterID] ON [dbo].[MissionAchieveLogs_20131007]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[MissionAchieveLogs_20131007]  WITH CHECK ADD  CONSTRAINT [CK_MissionAchieveLogs_20131007_AchieveDate] CHECK  (([AchieveDate]>='2013-10-07' AND [AchieveDate]<'2013-10-08'))
GO
ALTER TABLE [dbo].[MissionAchieveLogs_20131007] CHECK CONSTRAINT [CK_MissionAchieveLogs_20131007_AchieveDate]
GO

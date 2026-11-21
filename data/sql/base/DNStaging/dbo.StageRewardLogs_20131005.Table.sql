USE [dnstaging]
GO
/****** Object:  Table [dbo].[StageRewardLogs_20131005]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[StageRewardLogs_20131005](
	[StageStartDate] [datetime2](0) NOT NULL,
	[StageEndDate] [datetime2](0) NULL,
	[StageStartID] [bigint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[RewardBoxCode] [tinyint] NULL,
	[RewardItemSerial] [bigint] NULL,
	[RewardItemID] [int] NULL,
	[WorldID] [int] NULL,
	[RewardFailReason] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_StageRewardLogs_20131005_StageStartDate]    Script Date: 11/14/2026 9:00:07 AM ******/
CREATE CLUSTERED INDEX [IX_NC_StageRewardLogs_20131005_StageStartDate] ON [dbo].[StageRewardLogs_20131005]
(
	[StageStartDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[StageRewardLogs_20131005]  WITH CHECK ADD  CONSTRAINT [CK_StageRewardLogs_20131005_StageStartDate] CHECK  (([StageStartDate]>='2013-10-05' AND [StageStartDate]<'2013-10-06'))
GO
ALTER TABLE [dbo].[StageRewardLogs_20131005] CHECK CONSTRAINT [CK_StageRewardLogs_20131005_StageStartDate]
GO

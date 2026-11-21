USE [dnstaging]
GO
/****** Object:  Table [dbo].[StageRewardLogs_20130921]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[StageRewardLogs_20130921](
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
/****** Object:  Index [IX_NC_StageRewardLogs_20130921_StageStartDate]    Script Date: 11/14/2026 9:00:07 AM ******/
CREATE CLUSTERED INDEX [IX_NC_StageRewardLogs_20130921_StageStartDate] ON [dbo].[StageRewardLogs_20130921]
(
	[StageStartDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[StageRewardLogs_20130921]  WITH CHECK ADD  CONSTRAINT [CK_StageRewardLogs_20130921_StageStartDate] CHECK  (([StageStartDate]>='2013-09-21' AND [StageStartDate]<'2013-09-22'))
GO
ALTER TABLE [dbo].[StageRewardLogs_20130921] CHECK CONSTRAINT [CK_StageRewardLogs_20130921_StageStartDate]
GO

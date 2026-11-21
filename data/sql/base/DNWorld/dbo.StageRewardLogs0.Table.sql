USE [dnworld]
GO
/****** Object:  Table [dbo].[StageRewardLogs0]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[StageRewardLogs0](
	[StageStartID] [bigint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[RewardBoxCode] [tinyint] NOT NULL,
	[RewardItemSerial] [bigint] NULL,
	[RewardItemID] [int] NOT NULL,
	[RewardFailReason] [int] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_StageRewardLogs0_StageStartID_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_StageRewardLogs0_StageStartID_CharacterID] ON [dbo].[StageRewardLogs0]
(
	[StageStartID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

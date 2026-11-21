USE [dnmembership]
GO
/****** Object:  Table [dbo].[EventRewardTargets]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[EventRewardTargets]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[EventRewardTargets](
	[EventRewardID] [int] NOT NULL,
	[EventRewardTargetID] [int] IDENTITY(1,1) NOT NULL,
	[AccountID] [int] NOT NULL,
	[CharacterID] [bigint] NULL,
	[DeleteFlag] [bit] NOT NULL,
 CONSTRAINT [PK_EventRewardTargets] PRIMARY KEY CLUSTERED 
(
	[EventRewardID] ASC,
	[EventRewardTargetID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NN_EventRewardTargets_AccountID_EventRewardID_DeleteFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[EventRewardTargets]') AND name = N'IX_NN_EventRewardTargets_AccountID_EventRewardID_DeleteFlag')
CREATE NONCLUSTERED INDEX [IX_NN_EventRewardTargets_AccountID_EventRewardID_DeleteFlag] ON [dbo].[EventRewardTargets]
(
	[AccountID] ASC,
	[EventRewardID] ASC,
	[DeleteFlag] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_EventRewardTargets_CharacterID_EventRewardID_DeleteFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[EventRewardTargets]') AND name = N'IX_NN_EventRewardTargets_CharacterID_EventRewardID_DeleteFlag')
CREATE NONCLUSTERED INDEX [IX_NN_EventRewardTargets_CharacterID_EventRewardID_DeleteFlag] ON [dbo].[EventRewardTargets]
(
	[CharacterID] ASC,
	[EventRewardID] ASC,
	[DeleteFlag] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_EventRewards_EventRewardTargets_EventRewardID]') AND parent_object_id = OBJECT_ID(N'[dbo].[EventRewardTargets]'))
ALTER TABLE [dbo].[EventRewardTargets]  WITH CHECK ADD  CONSTRAINT [FK_EventRewards_EventRewardTargets_EventRewardID] FOREIGN KEY([EventRewardID])
REFERENCES [dbo].[EventRewards] ([EventRewardID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_EventRewards_EventRewardTargets_EventRewardID]') AND parent_object_id = OBJECT_ID(N'[dbo].[EventRewardTargets]'))
ALTER TABLE [dbo].[EventRewardTargets] CHECK CONSTRAINT [FK_EventRewards_EventRewardTargets_EventRewardID]
GO

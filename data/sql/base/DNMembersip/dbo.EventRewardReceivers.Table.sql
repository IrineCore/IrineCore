USE [dnmembership]
GO
/****** Object:  Table [dbo].[EventRewardReceivers]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[EventRewardReceivers]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[EventRewardReceivers](
	[CharacterID] [bigint] NOT NULL,
	[EventRewardID] [int] NOT NULL,
	[AccountID] [int] NOT NULL,
	[ReceiveDate] [smalldatetime] NOT NULL,
 CONSTRAINT [PK_EventRewardReceivers] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[EventRewardID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NN_EventRewardReceivers_AccountID_EventRewardID]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[EventRewardReceivers]') AND name = N'IX_NN_EventRewardReceivers_AccountID_EventRewardID')
CREATE NONCLUSTERED INDEX [IX_NN_EventRewardReceivers_AccountID_EventRewardID] ON [dbo].[EventRewardReceivers]
(
	[AccountID] ASC,
	[EventRewardID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_EventRewards_EventRewardReceivers_EventRewardID]') AND parent_object_id = OBJECT_ID(N'[dbo].[EventRewardReceivers]'))
ALTER TABLE [dbo].[EventRewardReceivers]  WITH CHECK ADD  CONSTRAINT [FK_EventRewards_EventRewardReceivers_EventRewardID] FOREIGN KEY([EventRewardID])
REFERENCES [dbo].[EventRewards] ([EventRewardID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_EventRewards_EventRewardReceivers_EventRewardID]') AND parent_object_id = OBJECT_ID(N'[dbo].[EventRewardReceivers]'))
ALTER TABLE [dbo].[EventRewardReceivers] CHECK CONSTRAINT [FK_EventRewards_EventRewardReceivers_EventRewardID]
GO

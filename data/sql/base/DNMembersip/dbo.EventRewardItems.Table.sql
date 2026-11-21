USE [dnmembership]
GO
/****** Object:  Table [dbo].[EventRewardItems]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[EventRewardItems]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[EventRewardItems](
	[EventRewardID] [int] NOT NULL,
	[RewardItemSequenceNumber] [int] IDENTITY(1,1) NOT NULL,
	[ProductFlag] [bit] NOT NULL,
	[ItemID] [bigint] NOT NULL,
	[ItemCount] [smallint] NULL,
	[ItemDurability] [smallint] NULL,
	[RandomSeed] [int] NULL,
	[ItemLevel] [tinyint] NULL,
	[ItemPotential] [tinyint] NULL,
	[SoulBoundFlag] [bit] NULL,
	[SealCount] [tinyint] NULL,
	[ItemOption] [tinyint] NULL,
	[ItemLifespan] [int] NULL,
	[EternityFlag] [bit] NULL,
	[ItemExpireDate] [smalldatetime] NULL,
	[DragonJewelType] [tinyint] NULL,
 CONSTRAINT [PK_EventRewardItems] PRIMARY KEY CLUSTERED 
(
	[EventRewardID] ASC,
	[RewardItemSequenceNumber] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_EventRewards_EventRewardItems_EventRewardID]') AND parent_object_id = OBJECT_ID(N'[dbo].[EventRewardItems]'))
ALTER TABLE [dbo].[EventRewardItems]  WITH CHECK ADD  CONSTRAINT [FK_EventRewards_EventRewardItems_EventRewardID] FOREIGN KEY([EventRewardID])
REFERENCES [dbo].[EventRewards] ([EventRewardID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_EventRewards_EventRewardItems_EventRewardID]') AND parent_object_id = OBJECT_ID(N'[dbo].[EventRewardItems]'))
ALTER TABLE [dbo].[EventRewardItems] CHECK CONSTRAINT [FK_EventRewards_EventRewardItems_EventRewardID]
GO

USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtEnchant]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtEnchant](
	[id] [int] NULL,
	[_EnchantID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EnchantLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EnchantRatio] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_BreakRatio] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MinDown] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MaxDown] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedItemID1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedItemCount1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedItemID2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedItemCount2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedItemID3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedItemCount3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedItemID4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedItemCount4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedItemID5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedItemCount5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedCoin] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State1Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State2Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State3Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State4Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State5Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State6] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State6Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State7] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State7Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State8] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State8Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State9] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State9Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State10] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_State10Value] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_FailResultDropItemTableID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_PotentialID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_PotentialRatio] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ActionChangeValue] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ProtectItemCount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ShieldItemCount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_DisjointDrop] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtEnchant_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtEnchant_id] ON [dbo].[ExtEnchant]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

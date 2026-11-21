USE [dnstaging]
GO
/****** Object:  Table [dbo].[ItemCountMonitoring]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemCountMonitoring](
	[BasisDate] [date] NOT NULL,
	[WorldID] [int] NOT NULL,
	[MaterializedItemID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemRemainCount] [smallint] NOT NULL,
	[ItemLevel] [tinyint] NOT NULL,
	[OwnerCharacterID] [bigint] NULL,
	[SoulBoundFlag] [bit] NOT NULL,
	[SealCount] [tinyint] NOT NULL,
	[ItemOption] [tinyint] NOT NULL,
 CONSTRAINT [PK_ItemCountMonitoring] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[WorldID] ASC,
	[MaterializedItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [OldData]
) ON [OldData]
GO
/****** Object:  Index [IX_NN_ItemCountMonitoring_ItemID_OwnerCharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemCountMonitoring_ItemID_OwnerCharacterID] ON [dbo].[ItemCountMonitoring]
(
	[ItemID] ASC,
	[OwnerCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [OldData]
GO

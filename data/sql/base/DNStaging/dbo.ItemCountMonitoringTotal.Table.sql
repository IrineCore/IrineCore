USE [dnstaging]
GO
/****** Object:  Table [dbo].[ItemCountMonitoringTotal]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemCountMonitoringTotal](
	[BasisDate] [date] NOT NULL,
	[WorldID] [int] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemLevel] [tinyint] NOT NULL,
	[ItemCount] [bigint] NOT NULL,
 CONSTRAINT [PK_ItemCountMonitoringTotal] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[WorldID] ASC,
	[ItemID] ASC,
	[ItemLevel] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [OldData]
) ON [OldData]
GO
/****** Object:  Index [IX_NN_ItemCountMonitoringTotal_ItemID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemCountMonitoringTotal_ItemID] ON [dbo].[ItemCountMonitoringTotal]
(
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [OldData]
GO

USE [dnstaging]
GO
/****** Object:  Table [dbo].[SelectedItems]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SelectedItems](
	[SelectedItemID] [bigint] NOT NULL,
	[PurchaseOrderDetailID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemOption] [tinyint] NULL,
 CONSTRAINT [PK_SelectedItems] PRIMARY KEY CLUSTERED 
(
	[SelectedItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_SelectedItems_PurchaseOrderDetailID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_SelectedItems_PurchaseOrderDetailID] ON [dbo].[SelectedItems]
(
	[PurchaseOrderDetailID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

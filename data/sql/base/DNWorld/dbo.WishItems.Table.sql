USE [dnworld]
GO
/****** Object:  Table [dbo].[WishItems]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[WishItems](
	[WishItemID] [int] IDENTITY(1,1) NOT NULL,
	[WishProductID] [int] NOT NULL,
	[MailID] [bigint] NOT NULL,
	[ProductID] [int] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemOption] [tinyint] NOT NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_WishItems_MailID]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_WishItems_MailID] ON [dbo].[WishItems]
(
	[WishProductID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_WishItems]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[WishItems] ADD  CONSTRAINT [PK_WishItems] PRIMARY KEY NONCLUSTERED 
(
	[WishItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
ALTER TABLE [dbo].[WishItems]  WITH CHECK ADD  CONSTRAINT [FK_Mails_WishItems_MailID] FOREIGN KEY([MailID])
REFERENCES [dbo].[Mails] ([MailID])
GO
ALTER TABLE [dbo].[WishItems] CHECK CONSTRAINT [FK_Mails_WishItems_MailID]
GO
ALTER TABLE [dbo].[WishItems]  WITH CHECK ADD  CONSTRAINT [FK_WishProducts_WishItems_WishProductID] FOREIGN KEY([WishProductID])
REFERENCES [dbo].[WishProducts] ([WishProductID])
GO
ALTER TABLE [dbo].[WishItems] CHECK CONSTRAINT [FK_WishProducts_WishItems_WishProductID]
GO
